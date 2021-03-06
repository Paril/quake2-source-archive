#include "m_turret.h"
#include "g_local.h"

#define	TURRET_WAIT	10

void turret_attack (edict_t *self);
void turret_fire (edict_t *self);
void turret_waitstart (edict_t *self);
void turret_waitloop (edict_t *self);
void turret_stand (edict_t *self);
void turret_idle (edict_t *self);
mmove_t turret_move_attack;
mmove_t turret_move_waitstart;
mmove_t turret_move_waitloop;



mframe_t turret_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t turret_move_stand = {FRAME_stand1, FRAME_stand1, turret_frames_stand, turret_idle};
mmove_t turret_move_idle = {FRAME_stand1, FRAME_stand1, turret_frames_stand, turret_stand};
mmove_t turret_move_sight = {FRAME_stand1, FRAME_stand1, turret_frames_stand, turret_waitstart};


void turret_stand (edict_t *self)
{
	self->takedamage = false;
	self->monsterinfo.currentmove = &turret_move_stand;
}

void turret_idle (edict_t *self)
{
	self->monsterinfo.currentmove = &turret_move_idle;
}

void turret_sight (edict_t *self, edict_t *other)
{
	self->monsterinfo.currentmove = &turret_move_sight;
}


mmove_t turret_move_waitstart = {FRAME_stand1, FRAME_stand1, turret_frames_stand, turret_waitloop};
mmove_t turret_move_waitloop = {FRAME_stand1, FRAME_stand1, turret_frames_stand, turret_waitstart};

void turret_waitstart (edict_t *self)
{
	self->monsterinfo.currentmove = &turret_move_waitstart;
}

void turret_waitloop (edict_t *self)
{
	self->style--;

	if (self->style == 0)
	{
		self->style = TURRET_WAIT;
		self->monsterinfo.currentmove = &turret_move_attack;
	}
	else
		self->monsterinfo.currentmove = &turret_move_waitloop;
}


mmove_t turret_move_pain = {FRAME_stand1, FRAME_stand1, turret_frames_stand, turret_waitstart};


void turret_fire (edict_t *self)
{

	if (self->enemy)
	{
		vec3_t	start;
		vec3_t	forward, right;
		vec3_t	end;
		vec3_t	dir, up;
		float r, u;

		int		effect;
		effect = 0;

		AngleVectors (self->s.angles, forward, right, up);
		G_ProjectSource (self->s.origin, monster_flash_offset[0], forward, right, start);

		VectorCopy (self->enemy->s.origin, end);
		end[2] += self->enemy->viewheight;
		//VectorSubtract (end, start, dir);

		r = crandom()*110;
		u = crandom()*60;
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		VectorSubtract (end, start, dir);
		VectorNormalize (dir);




		monster_fire_blaster (self, start, dir, 10, 650, false, effect);
	}
}


void turret_startfire (edict_t *self)
{
	self->takedamage = true;
}

void turret_endfire (edict_t *self)
{
	self->takedamage = false;
}

mframe_t turret_frames_attack [] =
{
	ai_stand, 0, NULL,
	ai_stand, 0, turret_startfire,
	ai_stand, 0, turret_fire,
	ai_stand, 0, turret_fire,
	ai_stand, 0, turret_fire,
	ai_stand, 0, turret_endfire
};
mmove_t turret_move_attack = {FRAME_stand1, FRAME_attack5, turret_frames_attack, turret_waitstart};


void turret_attack (edict_t *self)
{
	self->monsterinfo.currentmove = &turret_move_attack;
}




void turret_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	BecomeExplosion1(self);
}


void turret_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->style = TURRET_WAIT;
	self->takedamage = false;
	self->monsterinfo.currentmove = &turret_move_waitstart;
}


void SP_monster_turret (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	self->s.modelindex = gi.modelindex ("models/monsters/turret/tris.md2");
	VectorSet (self->mins, -10, -10, -10);
	VectorSet (self->maxs, 10, 10, 10);

	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;

	self->health = 25;
	self->mass = 200;

	//start with a shield on
	self->takedamage = false;

	self->style = TURRET_WAIT;

	self->pain = turret_pain;
	self->die = turret_die;

	self->monsterinfo.stand = turret_stand;
	self->monsterinfo.attack = turret_attack;
	self->monsterinfo.sight = turret_sight;
	self->monsterinfo.idle = turret_idle;
	self->monsterinfo.walk = turret_waitstart;
	self->monsterinfo.run = turret_waitstart;
	self->monsterinfo.melee = turret_waitstart;


	self->monsterinfo.currentmove = &turret_move_waitstart;
	self->monsterinfo.scale = MODEL_SCALE;

	gi.linkentity (self);

	//floats in air so keep starting position
	flymonster_start (self);
}
