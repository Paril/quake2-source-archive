/*
==============================================================================

flyer

==============================================================================
*/

#include "g_local.h"
#include "m_genq1scrag.h"

qboolean visible (edict_t *self, edict_t *other);
void q1_fire_acidspit (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);

static int	sound_sight;
static int	sound_idle1;
static int	sound_idle2;
static int  sound_attack;
static int	sound_die;
static int	sound_pain;
static int	sound_hit;


void scrag_sight (edict_t *self, edict_t *other)
{
	gi.sound (self, CHAN_VOICE, sound_sight, 1, ATTN_NORM, 0);
}

void scarg_attack_sound(edict_t *self)
{
	gi.sound (self, CHAN_VOICE, sound_attack, 1, ATTN_IDLE, 0);
}

static void scrag_idle (edict_t *self)
{
	if(random() < 0.2)
	{
		if(random() <0.5)
		gi.sound (self, CHAN_VOICE, sound_idle1, 1, ATTN_IDLE, 0);
		else
		gi.sound (self, CHAN_VOICE, sound_idle2, 1, ATTN_IDLE, 0);
	}
}


mframe_t scrag_frames_stand [] =
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
	ai_stand, 0, scrag_idle
};
mmove_t	scrag_move_stand = {FRAME_hover1, FRAME_hover15, scrag_frames_stand, NULL};

void scrag_stand (edict_t *self)
{
		self->monsterinfo.currentmove = &scrag_move_stand;
}


mframe_t scrag_frames_walk [] =
{
	ai_walk, 6, scrag_idle,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL,
	ai_walk, 6, NULL
};
mmove_t	scrag_move_walk = {FRAME_hover1, FRAME_hover15, scrag_frames_walk, NULL};

void scrag_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &scrag_move_walk;
}


void scrag_run (edict_t *self);

mframe_t scrag_frames_run [] =
{
	ai_run, 12, scrag_idle,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL,
	ai_run, 12, NULL
};
mmove_t	scrag_move_run = {FRAME_fly1, FRAME_fly14, scrag_frames_run, scrag_run};


void scrag_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &scrag_move_stand;
	else
		self->monsterinfo.currentmove = &scrag_move_run;
}






mframe_t scrag_frames_pain [] =
{	
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL
};
mmove_t scrag_move_pain = {FRAME_pain1, FRAME_pain4, scrag_frames_pain, scrag_run};


void scrag_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 2;
	if (skill->value == 3)
		return;		// no pain anims in nightmare

	gi.sound (self, CHAN_VOICE, sound_pain, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &scrag_move_pain;
}

void scrag_dead(edict_t * self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);

	M_FlyCheck (self);
}

mframe_t scrag_frames_die [] =
{	
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL,
		ai_move, 0, NULL
};
mmove_t scrag_move_die = {FRAME_death1, FRAME_death8, scrag_frames_die, scrag_dead};


void scrag_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
// check for gib

	if (self->health <= self->gib_health)
	{
		if(random() <0.5)
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/udeath.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (self, CHAN_VOICE, gi.soundindex ("q1guy/gib.wav"), 1, ATTN_NORM, 0);

		for (n= 0; n < 3; n++)
			ThrowGib (self, "models/objects/q1gibs/q1gib2/tris.md2", damage, GIB_ORGANIC);
		ThrowHead (self, "models/monsters/q1monstr/scrag/head/tris.md2", damage, GIB_ORGANIC);
		self->deadflag = DEAD_DEAD;
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;

// regular death
	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_YES;

	gi.sound (self, CHAN_VOICE, sound_die, 1, ATTN_NORM, 0);
	self->monsterinfo.currentmove = &scrag_move_die;
}


void scrag_fire (edict_t *self)
{
	vec3_t	forward, right, start;
	vec3_t	end, dir;

	if(!self->enemy)
		return;

	gi.sound (self, CHAN_VOICE, sound_attack, 1, ATTN_NORM, 0);

//fire 1	
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_FLYER_BLASTER_1], forward, right, start);
//
	start[2]+=32;

	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);
	VectorNormalize (dir);

/*	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_FLYER_BLASTER_1);
	gi.multicast (start, MULTICAST_PVS);
*/
	q1_fire_acidspit(self,start, dir, 10, 500);

// fire 2

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, monster_flash_offset[MZ2_FLYER_BLASTER_2], forward, right, start);
//
	start[2]+=32;
	
	VectorCopy (self->enemy->s.origin, end);
	end[2] += self->enemy->viewheight;
	VectorSubtract (end, start, dir);
	
	VectorNormalize (dir);
	
/*	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self - g_edicts);
	gi.WriteByte (MZ2_FLYER_BLASTER_2);
	gi.multicast (start, MULTICAST_PVS);
*/
	q1_fire_acidspit(self,start, dir, 10, 500);
}


mframe_t scrag_frames_attack [] =
{
		ai_charge, 0, NULL,
		ai_charge, 0, NULL,
		ai_charge, 0, scarg_attack_sound,					
		ai_charge, 0, NULL,				
		ai_charge, 0, NULL,			
		ai_charge, -1,scrag_fire,				
		ai_charge, -2,NULL,					
		ai_charge, -3,NULL,					
		ai_charge, -2,scrag_fire,			
		ai_charge, -1,NULL,					
		ai_charge, 0, scrag_fire,		
		ai_charge, 0, NULL,
		ai_charge, 0, NULL
};

mmove_t scrag_move_attack = {FRAME_magatt1, FRAME_magatt13, scrag_frames_attack, scrag_run};

void scrag_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &scrag_move_attack;
}


// QUAKE MONSTER SCRAG
qboolean monster_start (edict_t *self);

void SP_q1_monster_scrag (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	sound_sight = gi.soundindex ("q1monstr/scrag/wsight.wav");
	sound_idle1 = gi.soundindex ("q1monstr/scrag/widle1.wav");
	sound_idle2 = gi.soundindex ("q1monstr/scrag/widle2.wav");
	sound_pain = gi.soundindex ("q1monstr/scrag/wpain.wav");
	sound_die = gi.soundindex ("q1monstr/scrag/wdeath.wav");
	sound_attack = gi.soundindex ("q1monstr/scrag/wattack.wav");
	sound_hit = gi.soundindex ("q1monstr/scrag/hit.wav");
	
	self->s.modelindex = gi.modelindex ("models/monsters/q1monstr/scrag/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 40);
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 80;
	self->mass = 50;

	self->pain = scrag_pain;
	self->die = scrag_die;

	self->flags |= FL_Q1_MONSTER;

	self->monsterinfo.stand = scrag_stand;
	self->monsterinfo.walk = scrag_walk;
	self->monsterinfo.run = scrag_run;
	self->monsterinfo.attack = scrag_attack;
	self->monsterinfo.melee = NULL;
	self->monsterinfo.sight = scrag_sight;
	self->monsterinfo.idle = scrag_walk;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &scrag_move_stand;	
	self->monsterinfo.scale = MODEL_SCALE;
	
	
	self->flags |= FL_FLY;
	if (!self->yaw_speed)
		self->yaw_speed = 10;
	self->viewheight = 10;

	//monster_start (self);
	flymonster_start (self);
}
