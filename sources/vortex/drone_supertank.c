/*
==============================================================================

SUPERTANK

==============================================================================
*/

#include "g_local.h"
#include "m_supertank.h"

qboolean visible (edict_t *self, edict_t *other);

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);
void drone_ai_walk (edict_t *self, float dist);

static int	sound_pain1;
static int	sound_pain2;
static int	sound_pain3;
static int	sound_death;
static int	sound_search1;
static int	sound_search2;

static	int	tread_sound;

void BossExplode (edict_t *self);

void TreadSound (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, tread_sound, 1, ATTN_NORM, 0);
}

void supertankRocket (edict_t *self);
void supertankMachineGun (edict_t *self);
void supertank_reattack1(edict_t *self);

//
// stand
//

mframe_t supertank_frames_stand []=
{
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL,
	drone_ai_stand, 0, NULL
};
mmove_t	supertank_move_stand = {FRAME_stand_1, FRAME_stand_60, supertank_frames_stand, NULL};
	
void supertank_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &supertank_move_stand;
}


mframe_t supertank_frames_run [] =
{
	drone_ai_run, 12,	TreadSound,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL,
	drone_ai_run, 12,	NULL
};
mmove_t	supertank_move_run = {FRAME_forwrd_1, FRAME_forwrd_18, supertank_frames_run, NULL};

//
// walk
//


mframe_t supertank_frames_forward [] =
{
	drone_ai_walk, 4,	TreadSound,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL,
	drone_ai_walk, 4,	NULL
};
mmove_t	supertank_move_forward = {FRAME_forwrd_1, FRAME_forwrd_18, supertank_frames_forward, NULL};

void supertank_forward (edict_t *self)
{
		self->monsterinfo.currentmove = &supertank_move_forward;
}

void supertank_walk (edict_t *self)
{
	if (!self->goalentity)
		self->goalentity = world;

	self->monsterinfo.currentmove = &supertank_move_forward;
}

void supertank_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &supertank_move_stand;
	else
		self->monsterinfo.currentmove = &supertank_move_run;
}

mframe_t supertank_frames_turn_right [] =
{
	ai_move,	0,	TreadSound,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t supertank_move_turn_right = {FRAME_right_1, FRAME_right_18, supertank_frames_turn_right, supertank_run};

mframe_t supertank_frames_turn_left [] =
{
	ai_move,	0,	TreadSound,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t supertank_move_turn_left = {FRAME_left_1, FRAME_left_18, supertank_frames_turn_left, supertank_run};

mframe_t supertank_frames_death1 [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	BossExplode
};
mmove_t supertank_move_death = {FRAME_death_1, FRAME_death_24, supertank_frames_death1, NULL};

mframe_t supertank_frames_backward[] =
{
	drone_ai_walk, 0,	TreadSound,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL,
	drone_ai_walk, 0,	NULL
};
mmove_t	supertank_move_backward = {FRAME_backwd_1, FRAME_backwd_18, supertank_frames_backward, NULL};

mframe_t supertank_frames_attack3[]=
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t supertank_move_attack3 = {FRAME_attak3_1, FRAME_attak3_27, supertank_frames_attack3, supertank_run};

void supertank_rerocket(edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && entdist(self, self->enemy) <= 512 && random() <= 0.8)
		self->s.frame = 25;

	self->monsterinfo.attack_finished = level.time + 2.0;
}

mframe_t supertank_frames_attack2[]=
{
	//ai_charge,	0,	NULL,//20
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	//ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	supertankRocket,//27
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	supertankRocket,//30
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	supertankRocket,//33
	ai_charge,	0,	supertank_rerocket,//GHz
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,//40
	//ai_move,	0,	NULL,
	//ai_move,	0,	NULL,
	//ai_move,	0,	NULL,
	//ai_move,	0,	NULL,
	//ai_move,	0,	NULL,
	//ai_move,	0,	NULL
};
mmove_t supertank_move_attack2 = {FRAME_attak2_6, FRAME_attak2_21, supertank_frames_attack2, supertank_run};

mframe_t supertank_frames_attack1[]=
{
	ai_charge,	0,	supertankMachineGun,
	ai_charge,	0,	supertankMachineGun,
	ai_charge,	0,	supertankMachineGun,
	ai_charge,	0,	supertankMachineGun,
	ai_charge,	0,	supertankMachineGun,
	ai_charge,	0,	supertankMachineGun,

};
mmove_t supertank_move_attack1 = {FRAME_attak1_1, FRAME_attak1_6, supertank_frames_attack1, supertank_reattack1};

mframe_t supertank_frames_end_attack1[]=
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t supertank_move_end_attack1 = {FRAME_attak1_7, FRAME_attak1_20, supertank_frames_end_attack1, supertank_run};
/*
void fire_mirv_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float radius, int speed, float timer);

void supertankMirv (edict_t *self)
{
	vec3_t	forward, start;
	int		damage;

	damage = 50 + 10*self->monsterinfo.level;

	MonsterAim(self, 0.5, 400, true, 0, forward, start);

	fire_mirv_grenade(self, start, forward, damage, 150, 400, 3.0);
}

void supertank_reattack4(edict_t *self);

mframe_t supertank_frames_attack4[]=
{
	ai_charge,	0,	supertankMirv,//74
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	supertankMirv,//79
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,

};
mmove_t supertank_move_attack4 = {FRAME_attak4_1, FRAME_attak4_6, supertank_frames_attack4, supertank_reattack4};

void supertank_reattack4(edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && random() <= 0.5)
		self->monsterinfo.currentmove = &supertank_move_attack4;
	self->monsterinfo.attack_finished = level.time + 1.0;
}
*/
void supertank_reattack1(edict_t *self)
{
	if (G_ValidTarget(self, self->enemy, true) && random() <= 0.9)
		self->monsterinfo.currentmove = &supertank_move_attack1;
	else
		self->monsterinfo.currentmove = &supertank_move_end_attack1;
	self->monsterinfo.attack_finished = level.time + 1.0;
}

void supertankRocket (edict_t *self)
{
	vec3_t	forward, start;
	int		damage, speed, flash_number;

	if (self->s.frame == FRAME_attak2_8)
		flash_number = MZ2_SUPERTANK_ROCKET_1;
	else if (self->s.frame == FRAME_attak2_11)
		flash_number = MZ2_SUPERTANK_ROCKET_2;
	else // (self->s.frame == FRAME_attak2_14)
		flash_number = MZ2_SUPERTANK_ROCKET_3;

	damage = 50 + 10*self->monsterinfo.level;
	speed = 650 + 30*self->monsterinfo.level;

	MonsterAim(self, 0.5, speed, true, flash_number, forward, start);

	monster_fire_rocket (self, start, forward, damage, speed, flash_number);
}	

void supertankMachineGun (edict_t *self)
{
	vec3_t	forward, start;
	int		damage, flash_number;

	flash_number = MZ2_SUPERTANK_MACHINEGUN_1 + (self->s.frame - FRAME_attak1_1);

	damage = 20 + 2*self->monsterinfo.level;

	MonsterAim(self, 0.8, 0, false, flash_number, forward, start);

	monster_fire_bullet (self, start, forward, damage, damage, 
		DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);
}	


void supertank_attack(edict_t *self)
{
	float	range = entdist(self, self->enemy);
	float	r = random();

	// medium range
	if (range <= 512)
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &supertank_move_attack1;
		//else if (r <= 0.4)
		//	self->monsterinfo.currentmove = &supertank_move_attack4;
		else
			self->monsterinfo.currentmove = &supertank_move_attack2;
	}
	// long range
	else
	{
		if (r <= 0.2)
			self->monsterinfo.currentmove = &supertank_move_attack2;
		else
			self->monsterinfo.currentmove = &supertank_move_attack1;
	}

	self->monsterinfo.attack_finished = level.time + 1.0;
}


//
// death
//

void BossExplode (edict_t *self)
{
	vec3_t	org;
	int		n;

	self->think = BossExplode;
	VectorCopy (self->s.origin, org);
	org[2] += 24 + (rand()&15);
	switch (self->count++)
	{
	case 0:
		org[0] -= 24;
		org[1] -= 24;
		break;
	case 1:
		org[0] += 24;
		org[1] += 24;
		break;
	case 2:
		org[0] += 24;
		org[1] -= 24;
		break;
	case 3:
		org[0] -= 24;
		org[1] += 24;
		break;
	case 4:
		org[0] -= 48;
		org[1] -= 48;
		break;
	case 5:
		org[0] += 48;
		org[1] += 48;
		break;
	case 6:
		org[0] -= 48;
		org[1] += 48;
		break;
	case 7:
		org[0] += 48;
		org[1] -= 48;
		break;
	case 8:
		self->s.sound = 0;
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", 500, GIB_ORGANIC);
		for (n= 0; n < 8; n++)
			ThrowGib (self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC);
		ThrowGib (self, "models/objects/gibs/chest/tris.md2", 500, GIB_ORGANIC);
		ThrowHead (self, "models/objects/gibs/gear/tris.md2", 500, GIB_METALLIC);
		self->deadflag = DEAD_DEAD;
		M_Remove(self, false, false);
		return;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (org);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->nextthink = level.time + 0.1;
}


void supertank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->count = 0;
	self->monsterinfo.currentmove = &supertank_move_death;
}

//
// monster_supertank
//

/*QUAKED monster_supertank (1 .5 0) (-64 -64 0) (64 64 72) Ambush Trigger_Spawn Sight
*/

void supertank_sight (edict_t *self, edict_t *other)
{
	if (random() > 0.5)
		gi.sound (self, CHAN_VOICE, sound_search1, 1, ATTN_NORM, 0);
	else
		gi.sound (self, CHAN_VOICE, sound_search2, 1, ATTN_NORM, 0);
}

void init_drone_supertank (edict_t *self)
{
	sound_death = gi.soundindex ("bosstank/btkdeth1.wav");
	sound_search1 = gi.soundindex ("bosstank/btkunqv1.wav");
	sound_search2 = gi.soundindex ("bosstank/btkunqv2.wav");
	tread_sound = gi.soundindex ("bosstank/btkengn1.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->mtype = M_SUPERTANK;
	self->monsterinfo.control_cost = 101;
	self->monsterinfo.cost = 300;
	self->s.modelindex = gi.modelindex ("models/monsters/boss1/tris.md2");
	VectorSet (self->mins, -64, -64, 0);
	VectorSet (self->maxs, 64, 64, 112);

	self->health = self->max_health = 20000*self->monsterinfo.level;
	self->gib_health = -500;
	self->mass = 800;

	self->die = supertank_die;
	self->monsterinfo.stand = supertank_stand;
	self->monsterinfo.walk = supertank_walk;
	self->monsterinfo.run = supertank_run;
	self->monsterinfo.attack = supertank_attack;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;
	self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	self->monsterinfo.currentmove = &supertank_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
	self->monsterinfo.sight = supertank_sight;

	self->nextthink = level.time + FRAMETIME;
	gi.linkentity (self);

	G_PrintGreenText(va("A level %d super tank has spawned!", self->monsterinfo.level));
}
