#include "g_local.h"

void laser_think (edict_t *self)
{
	
	if (!self->owner || self->delay < level.time)
	{
		G_FreeEdict(self);
		return;	
	}
	
	self->nextthink = level.time + FRAMETIME;
}

void laser_start (edict_t *ent, vec3_t start, vec3_t aimdir, int	color, float laserend, float delete_time, int width)
{
	edict_t *self;
	vec3_t forward, end;

	
	self = G_Spawn();

	VectorCopy (start, self->s.origin);
	
	AngleVectors(aimdir,forward,NULL,NULL);
    VectorScale(forward,laserend,forward);
    VectorAdd(start, forward, end);
	VectorCopy(end, self->s.old_origin); 

	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;
	self->owner = ent;
	self->delay = level.time + delete_time;
	self->spawnflags=laserend;

	self->s.frame = width;//4;

	// set the color
	self->s.skinnum = color;

	self->think = laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;

	gi.linkentity (self);

	laser_think (self);
}

