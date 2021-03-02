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

rc_car

==============================================================================
*/

#include "g_local.h"
#include "stdio.h"

mframe_t rc_car_frames_stand [] =
{
	ai_stand, 0, NULL
};
mmove_t rc_car_move_stand = {0, 0, rc_car_frames_stand, NULL};

void rc_car_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &rc_car_move_stand;
}


mframe_t rc_car_frames_run [] =
{
	ai_run, 5,	NULL
};
mmove_t rc_car_move_run = {0, 0, rc_car_frames_run, NULL};

void rc_car_run (edict_t *self)
{
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		self->monsterinfo.currentmove = &rc_car_move_stand;
	else
		self->monsterinfo.currentmove = &rc_car_move_run;
}

void rc_car_pain (edict_t *self, edict_t *other, float kick, int damage)
{	
}

mframe_t rc_car_frames_death [] =
{
	ai_move, 0, NULL
};
mmove_t rc_car_move_death = {0, 0, rc_car_frames_death, NULL};

void rc_car_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// Make ent no longer 'Transformed';
	self->transformer->s.modelindex = 255;
	self->transformer->is_transformed=false;
	//self->transformer->health=self->old_health;
	//self->transformer->movetype=MOVETYPE_WALK;
	//self->transformer->svflags &= ~SVF_NOCLIENT;
	//self->transformer->client->ps.gunindex = self->transformer->client->old_gunindex;
	//VectorCopy (self->transformer->s.origin, self->transformer->client->old_origin);
	//gi.linkentity (self->transformer);
	self->transformer->vehicle=NULL;
	self->transformer->chasetarget = NULL;
	self->transformer->movetype = MOVETYPE_WALK;

	gi.sound (self, CHAN_VOICE, SoundIndex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	for (n= 0; n < 2; n++)
		ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
	for (n= 0; n < 4; n++)
		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
	self->deadflag = DEAD_DEAD;
	return;
}


mframe_t rc_car_up [] =
{
	ai_move, /*15*/30,	NULL
};
mmove_t CarUp = {0, 0, rc_car_up, NULL};

mframe_t rc_car_down [] =
{
	ai_move, -/*15*/30,	NULL
};
mmove_t CarDown = {0, 0, rc_car_down, NULL};

mframe_t rc_car_stop [] =
{
	ai_move, 0,	NULL
};
mmove_t CarStop = {0, 0, rc_car_stop, NULL};

void RCCarUp(edict_t *self)
{
	self->monsterinfo.currentmove = &CarUp;
}
void RCCarDown(edict_t *self)
{
	self->monsterinfo.currentmove = &CarDown;
}
void RCCarStop(edict_t *self)
{
	self->monsterinfo.currentmove = &CarStop;
}

/*QUAKED monster_rc_car (1 .5 0) (-32 -32 -24) (32 32 64) Ambush Trigger_Spawn Sight
*/
void SP_monster_rc_car (edict_t *self)
{
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = ModelIndex ("models/other/car/tris.md2");
	VectorSet (self->mins, -9, -9, -6);
	VectorSet (self->maxs, 9, 9, 6);
	self->s.renderfx |= RF_IR_VISIBLE;
	self->health = 150;
	self->gib_health = 0;
	self->mass = 200;
	self->monsterinfo.aiflags |= AI_CAR;

	self->pain = rc_car_pain;
	self->die = rc_car_die;
	self->classname = "monster_car";

	self->monsterinfo.stand = rc_car_stand;
	self->monsterinfo.run = rc_car_run;
	self->monsterinfo.dodge = NULL;

	gi.linkentity (self);
	self->monsterinfo.currentmove = &rc_car_move_stand;
	self->monsterinfo.scale = 1.0;

	walkmonster_start (self);
}
