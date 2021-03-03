/*==============================================================================
The Weapons Factory - 
Earthquake Function
Original code by Gregg Reno
==============================================================================*/
/* 
   This command causes an earthquake
*/

#include "g_local.h"

void wf_earthquake_think (edict_t *self)
{
	int		i;
	edict_t	*e;

	if (self->last_move_time < level.time)
	{
		gi.positioned_sound (self->s.origin, self, CHAN_AUTO, self->noise_index, 1.0, ATTN_NONE, 0);
		self->last_move_time = level.time + 0.5;

	}

	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		if (!e->groundentity)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict (self);

}


//Set up an earthquake entity
void wf_earthquake (edict_t *owner)
{
    edict_t *self;

	self = G_Spawn();

	VectorCopy(owner->s.origin, self->s.origin);

	//set the team
//	self->wf_team = owner->client->resp.ctf_team;
    self->count = 3;        //how long it will last (3 seconds?)
	self->speed = 200;
	self->svflags |= SVF_NOCLIENT;
	self->think = wf_earthquake_think;
	self->timestamp = level.time + self->count;
	self->nextthink = level.time + FRAMETIME;
	self->last_move_time = 0;
	self->noise_index = gi.soundindex ("world/quake.wav");
	gi.linkentity (self);

}
