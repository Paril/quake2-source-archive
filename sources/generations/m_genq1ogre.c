/*
==============================================================================

QUAKE OGRE BABY

==============================================================================
*/

#include "g_local.h"
#include "m_genq1ogre.h"

static int	sound_pain;
static int	sound_death;
static int	sound_idle;
static int	sound_idle2;
static int	sound_wake;
static int	sound_saw;
static int	sound_drag;


void ogre_check_refire (edict_t *self);
void ogre_attack(edict_t *self);

static void ogre_idle_sound1 (edict_t *self)
{
	if(random() > .2)
		gi.sound (self, CHAN_VOICE, sound_idle, 1, ATTN_IDLE, 0);
}

static void ogre_idle_sound2 (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_idle2, 1, ATTN_IDLE, 0);
}

void ogre_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_wake, 1, ATTN_NORM, 0);
	ogre_attack(self);
}

static void ogre_drag_sound (edict_t *self)
{
	//if(anglemod(self->s.angles[YAW]) != self->ideal_yaw)
	if(random() < 0.2)
		gi.sound (self, CHAN_VOICE, sound_drag, 1, ATTN_STATIC, 0);
}



void ogre_stand (edict_t *self);

mframe_t ogre_frames_stand [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, ogre_idle_sound1,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
	ai_stand, 0, NULL,
};

mmove_t	ogre_move_stand = {FRAME_stand1, FRAME_stand9, ogre_frames_stand, ogre_stand};

void ogre_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &ogre_move_stand;
}


mframe_t ogre_frames_walk [] =
{
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 2, ogre_idle_sound1,
	ai_walk, 2, NULL,
	ai_walk, 2, NULL,
	ai_walk, 6, ogre_drag_sound,
	ai_walk, 3, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 1, NULL,
	ai_walk, 2, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 3, NULL,
	ai_walk, 4, NULL
};
mmove_t ogre_move_walk = {FRAME_walk1, FRAME_walk16, ogre_frames_walk, NULL};

void ogre_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &ogre_move_walk;
}


mframe_t ogre_frames_run [] =
{
	ai_run, 9, NULL,
	ai_run, 12,NULL,
	ai_run, 8, NULL,
	ai_run, 22,NULL,
	ai_run, 16,NULL,
	ai_run, 4, NULL,
	ai_run, 13,ogre_attack,
	ai_run, 24,NULL
};

mmove_t ogre_move_run = {FRAME_run1, FRAME_run8, ogre_frames_run, NULL};


void ogre_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &ogre_move_stand;
	else
		self->monsterinfo.currentmove = &ogre_move_run;
}



mframe_t ogre_frames_pain1 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,	 NULL
};
mmove_t ogre_move_pain1 = {FRAME_pain1, FRAME_pain5, ogre_frames_pain1, ogre_run};

mframe_t ogre_frames_pain2 [] =
{
	ai_move, -1,NULL,
	ai_move, 0, NULL,
	ai_move, 1, NULL
};
mmove_t ogre_move_pain2 = {FRAME_painb1, FRAME_painb3, ogre_frames_pain2, ogre_run};


mframe_t ogre_frames_pain3 [] =
{
	ai_move, -3, NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,	 NULL
};
mmove_t ogre_move_pain3 = {FRAME_painc1, FRAME_painc6, ogre_frames_pain3, ogre_run};


mframe_t ogre_frames_pain4 [] =
{
	ai_move, -3, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 1,	 NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t ogre_move_pain4 = {FRAME_paind1, FRAME_paind16, ogre_frames_pain4, ogre_run};

mframe_t ogre_frames_pain5 [] =
{
	ai_move, -3, NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,  NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 1,  NULL,
	ai_move, 1,	 NULL,
	ai_move, 0,	 NULL,
};
mmove_t ogre_move_pain5 = {FRAME_paine1, FRAME_paine15, ogre_frames_pain5, ogre_run};




void ogre_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	float r;


	if (level.time < self->pain_debounce_time)
		return;

	r=random();

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);	

	if (r < 0.25)
	{
		self->monsterinfo.currentmove = &ogre_move_pain1;
		self->pain_debounce_time = level.time + 1;
		
	}
	else if (r < 0.5)
	{
		self->monsterinfo.currentmove = &ogre_move_pain2;
		self->pain_debounce_time = level.time + 1;
		
	}
	else if (r < 0.75)
	{
		self->monsterinfo.currentmove = &ogre_move_pain3;
		self->pain_debounce_time = level.time + 1;
		
	}
	else if (r < 0.88)
	{
		self->monsterinfo.currentmove = &ogre_move_pain4;
		self->pain_debounce_time = level.time + 2;
		
	}
	else
	{
		self->monsterinfo.currentmove =  &ogre_move_pain5;
		self->pain_debounce_time = level.time + 2;
		
	}
}


static void droprockets (edict_t *self)
{
	edict_t	*backpack;

	backpack = Drop_Item(self, FindItemByClassname("item_q1_backpack"));
	backpack->item = FindItemByClassname("ammo_rockets");
	backpack->count = 2;
	backpack->touch = Touch_Item;
	backpack->nextthink = level.time + 119;
	backpack->think = G_FreeEdict;
}

void ogre_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t ogre_frames_death1 [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -7, NULL,
	ai_move, -3, NULL,
	ai_move, -5, NULL,
	ai_move, 8,	 NULL,
	ai_move, 6,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t ogre_move_death1 = {FRAME_death1, FRAME_death14, ogre_frames_death1, ogre_dead};

mframe_t ogre_frames_death2 [] =
{
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, -7, droprockets,
	ai_move, -3, NULL,
	ai_move, -5, NULL,
	ai_move, 8,	 NULL,
	ai_move, 6,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL,
	ai_move, 0,	 NULL
};
mmove_t ogre_move_death2 = {FRAME_bdeath1, FRAME_bdeath10, ogre_frames_death2, ogre_dead};



void ogre_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

// check for gib
	if (self->health <= self->gib_health)
	{
		if(random() <0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/udeath.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/gib.wav"), 1, ATTN_NORM, 0);

		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib1/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib3/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1monstr/ogre/head/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	gi.sound (self, CHAN_VOICE, sound_death, 1, ATTN_NORM, 0);
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	if(random() < 0.5)
	self->monsterinfo.currentmove = &ogre_move_death1;
	else
	self->monsterinfo.currentmove = &ogre_move_death2;
}


void ogre_grenade_fire(edict_t *self)
{

	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	target;
	vec3_t	aim;
	
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_GUNNER_GRENADE_1], forward, right, start);

	// project enemy back a bit and target there
	VectorCopy (self->enemy->s.origin, target);
	
	//if(range(self,self->enemy) > RANGE_MID)
	VectorMA (target, -0.1, self->enemy->velocity, target);
	
	if(range(self,self->enemy) > RANGE_MID)
	target[2] += self->enemy->viewheight;
	else
	target[2] += self->enemy->viewheight*0.8;
	
	VectorSubtract (target, start, aim);
	VectorNormalize (aim);
	
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | 128);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	gi.sound (self, CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("q1weap/grenades/grenade.wav"), 1.0, ATTN_NORM, 0);
	q1_fire_grenade (self, start,aim,40 , 600, 2.5, 80);
}


//////////////
// Skid - add Gib fall off

void ogre_swing_left (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	fire_hit (self, aim, ((random() + random() + random()) * 4), 100);
}

void ogre_swing_right (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->maxs[0], 8);
	fire_hit (self, aim, ((random() + random() + random()) * 4), 100);
}

void ogre_smash (edict_t *self)
{
	vec3_t	aim;

	VectorSet (aim, MELEE_DISTANCE, self->mins[0], 8);
	if (fire_hit (self, aim, (25 + (rand() %5)), 100))
		gi.sound (self, CHAN_WEAPON, sound_saw, 1, ATTN_NORM, 0);
}


void ogre_check_refire (edict_t *self)
{
	if (!self->enemy || !self->enemy->inuse || self->enemy->health <= 0)
		return;

	if (skill->value == 3|| (range(self, self->enemy) == RANGE_MELEE))
	{
		if(random() > 0.5)
			self->monsterinfo.nextframe = FRAME_swing1;
		else
			self->monsterinfo.nextframe = FRAME_smash1;
	}
	else
		ogre_attack(self);
}


static void ogre_sawswingsound(edict_t *self)
{
	gi.sound (self, CHAN_WEAPON, sound_saw, 1, ATTN_NORM, 0);
}

mframe_t ogre_frames_swing [] =
{
	
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_sawswingsound,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_swing_right,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_sawswingsound,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_swing_left,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_check_refire
};
mmove_t ogre_move_swing_attack = {FRAME_swing1, FRAME_swing14, ogre_frames_swing, ogre_run};


mframe_t ogre_frames_smash [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_smash,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_check_refire
};
mmove_t ogre_move_smash_attack = {FRAME_smash1, FRAME_smash14, ogre_frames_smash, ogre_run};

mframe_t ogre_frames_attack_grenade [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL,
	ai_charge, 0, ogre_grenade_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL //ogre_attack
};
mmove_t ogre_move_attack_grenade = {FRAME_shoot1, FRAME_shoot6, ogre_frames_attack_grenade, ogre_run};


void ogre_attack(edict_t *self)
{
	int r;
	
	if(!self->enemy)
		return;

	r = range(self,self->enemy);

	if (r == RANGE_MELEE)
	{
		self->monsterinfo.currentmove = &ogre_move_swing_attack;
	}
	else if(visible(self,self->enemy) && infront(self,self->enemy)
		&& (r < RANGE_FAR) && !(self->monsterinfo.aiflags & AI_SOUND_TARGET))
	{
		self->monsterinfo.currentmove = &ogre_move_attack_grenade;
	}
	else
		self->monsterinfo.currentmove = &ogre_move_run;
}


void SP_q1_monster_ogre (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_pain =gi.soundindex ("q1monstr/ogre/ogpain1.wav");	
	sound_death=gi.soundindex ("q1monstr/ogre/ogdth.wav");	
	sound_idle =gi.soundindex ("q1monstr/ogre/ogidle.wav");	
	sound_idle2=gi.soundindex ("q1monstr/ogre/ogidle2.wav");	
	sound_wake =gi.soundindex ("q1monstr/ogre/ogwake.wav");	
	sound_saw  =gi.soundindex ("q1monstr/ogre/ogsawatk.wav");	
	sound_drag =gi.soundindex ("q1monstr/ogre/ogdrag.wav");	

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex ("models/monsters/q1monstr/ogre/tris.md2");
	VectorSet (self->mins, -20, -20, -24); //16 16
	VectorSet (self->maxs, 20, 20, 32); //16 16

	self->health = 200;
	self->gib_health = -80;
	self->mass = 400;

	self->pain = ogre_pain;
	self->die = ogre_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = ogre_stand;
	self->monsterinfo.walk = ogre_walk;
	self->monsterinfo.run = ogre_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = ogre_attack;
	self->monsterinfo.melee = ogre_check_refire;
	self->monsterinfo.sight = ogre_sight;
	self->monsterinfo.search = ogre_stand;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &ogre_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
