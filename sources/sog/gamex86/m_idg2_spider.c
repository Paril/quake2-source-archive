/*
==============================================================================

DOOM SPIDER MASTERMIND

==============================================================================
*/

#include "g_local.h"
#include "m_idg2_spider.h"

static int	sound_pain;
static int	sound_die;
static int	sound_sight;

void BossExplode (edict_t *self);    // from m_supertank.c

mframe_t idg2_spider_frames_stand [] =
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
	ai_stand, 0, NULL
};
mmove_t idg2_spider_move_stand = {FRAME_stand1, FRAME_stand10, idg2_spider_frames_stand, NULL};

void idg2_spider_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_spider_move_stand;
}

mframe_t idg2_spider_frames_walk [] =
{
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	
	ai_walk, 4, NULL,
	ai_walk, 5, NULL,
	ai_walk, 4, NULL,
	ai_walk, 4, NULL
};
mmove_t idg2_spider_move_walk = {FRAME_walk1, FRAME_walk9, idg2_spider_frames_walk, NULL};

void idg2_spider_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &idg2_spider_move_walk;
}

mframe_t idg2_spider_frames_run [] =
{
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 12, NULL,
	
	ai_run, 12, NULL,
	ai_run, 10, NULL,
	ai_run, 10, NULL,
	ai_run, 12, NULL
};
mmove_t idg2_spider_move_run = {FRAME_walk1, FRAME_walk9, idg2_spider_frames_run, NULL};

void idg2_spider_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &idg2_spider_move_stand;
	else
		self->monsterinfo.currentmove = &idg2_spider_move_run;
}


mframe_t idg2_spider_frames_pain [] =
{
	ai_move, -3, NULL,
	ai_move, -2, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL
};
mmove_t idg2_spider_move_pain = {FRAME_pain1, FRAME_pain4, idg2_spider_frames_pain, idg2_spider_run};

void idg2_spider_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 3;
	
	if (skill->value == 3)

		return;		// no pain anims in nightmare

	self->monsterinfo.currentmove = &idg2_spider_move_pain;
	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
}



void idg2_spider_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_BODY, sound_sight, 1, ATTN_NORM, 0);
}

mframe_t idg2_spider_frames_death [] =
{
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	
	ai_move, 0, NULL,
	ai_move, 0, NULL,
	ai_move, 0, BossExplode
};
mmove_t idg2_spider_move_death = {FRAME_death1, FRAME_death8, idg2_spider_frames_death, NULL};



void idg2_spider_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO;
	self->count = 0;
    self->monsterinfo.currentmove = &idg2_spider_move_death;
	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	
	// hack
	// otherwise player can't exit map
	
	if((strcmp("boss2", level.mapname)==0) && (self->target))
		G_UseTargets(self,self->enemy);
}

void idg2_spider_prepare_fire (edict_t *self)
{
	int		n;

	n = (rand() & 15) + 3 + 7;
	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

void idg2_spider_fire (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right;
	vec3_t	vec;
	int		flash_number;

	flash_number = MZ2_INFANTRY_MACHINEGUN_1;
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[flash_number], forward, right, start);

	if (self->enemy)
	{
		VectorMA (self->enemy->s.origin, -0.2, self->enemy->velocity, target);
		target[2] += self->enemy->viewheight;
		VectorSubtract (target, start, forward);
		VectorNormalize (forward);
	}
	else
	{
		AngleVectors (self->s.angles, forward, right, NULL);
	}

	monster_fire_bullet (self, start, forward, 11, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, flash_number);

	if (level.time >= self->monsterinfo.pausetime)
		self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
	else
		self->monsterinfo.aiflags |= AI_HOLD_FRAME;

}

mframe_t idg2_spider_frames_attack [] =
{
	ai_charge, 0, NULL,
	ai_charge, 0, idg2_spider_prepare_fire,
	ai_charge, 0, idg2_spider_fire,
	ai_charge, 0, NULL,
	ai_charge, 0, NULL
};
mmove_t idg2_spider_move_attack = {FRAME_attack1, FRAME_attack5, idg2_spider_frames_attack, idg2_spider_run};

void idg2_spider_attack(edict_t *self)
{
   self->monsterinfo.currentmove = &idg2_spider_move_attack;
}


/*QUAKED monster_idg2_spider (1 .5 0) (-96 -96 -24) (96 96 96) Ambush Trigger_Spawn Sight
DOOM Spider Mastermind
*/
void SP_monster_idg2_spider (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if ((int)sogflags->value & SOG_IDG1_ONLY)
	{
		self->s.origin[2] += 8;
		SP_monster_idg1_general (self);
		return;
	}

	sound_pain = gi.soundindex ("idg2monster/sarge/pain.wav");
	sound_die = gi.soundindex ("idg2monster/spider/death.wav");	
	sound_sight = gi.soundindex ("idg2monster/spider/sight.wav");

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/idg2/spider/tris.md2");
	VectorSet (self->mins, -96, -96, -24);
	VectorSet (self->maxs, 96, 96, 96);

	self->health = 1200;

	self->gib_health = -666;
	self->mass = 1000;

	self->flags |= (FL_D_MONSTER|FL_NO_KNOCKBACK);

	
	self->pain = idg2_spider_pain;
	self->die = idg2_spider_die;

	self->monsterinfo.stand = idg2_spider_stand;
	self->monsterinfo.walk = idg2_spider_walk;
	self->monsterinfo.run = idg2_spider_run;
	self->monsterinfo.dodge = NULL;
	self->monsterinfo.attack = idg2_spider_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = idg2_spider_sight;
	self->monsterinfo.idle = NULL;


	gi.linkentity (self);

	self->monsterinfo.currentmove = &idg2_spider_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;

	walkmonster_start (self);
}
