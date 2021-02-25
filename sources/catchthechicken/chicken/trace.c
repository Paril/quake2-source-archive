#include "g_local.h"
#include "a_bot.h"

static void bot_laser_on	(edict_t *self);
static void bot_laser_off	(edict_t *self);
static void bot_laser_think (edict_t *self);
static void bot_laser_use	(edict_t *self, edict_t *other, edict_t *activator);
static void bot_laser_start (edict_t *self);

static void bot_laser_on (edict_t *self)
{
	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	bot_laser_think (self);
}

static void bot_laser_off (edict_t *self)
{
	self->spawnflags &= ~1;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
}

static void bot_laser_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	VectorCopy (self->movedir, last_movedir);
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
	VectorSubtract (point, self->s.origin, self->movedir);
	VectorNormalize (self->movedir);
//	if (!VectorCompare(self->movedir, last_movedir))
//		self->spawnflags |= 0x80000000;

	ignore = self;
	VectorCopy (self->owner->s.origin,	self->s.origin);
	VectorCopy (self->s.origin,			start);
	VectorCopy (self->enemy->s.origin,	end);

	//
	// Offset beam so we dont get them overlapping
	//
	end[2]   += (self->flags * 5);
	start[2] += (self->flags * 5);

	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			break;

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}

	VectorCopy (tr.endpos, self->s.old_origin);
	self->nextthink = level.time + FRAMETIME;
}

static void bot_laser_use (edict_t *self, edict_t *other, edict_t *activator)
{
//	if (self->spawnflags & 1)
//		bot_laser_off (self);
//	else
		bot_laser_on (self);
}

static void bot_laser_start (edict_t *self)
{
	self->movetype		= MOVETYPE_NONE;
	self->solid			= SOLID_NOT;
	self->s.renderfx	|= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex	= 1;			// must be non-zero
	self->s.frame		= 2;			// Diameter of beam

	self->use   = bot_laser_use;
	self->think = bot_laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	if (self->spawnflags & 1)
		bot_laser_on (self);
	else
		bot_laser_off (self);
}

void bot_BeamCreate (int beamId, edict_t *self, edict_t *enemy, int colour)
{
	edict_t *beam;

	if (beamId < BOT_MAXBEAMS)
	{
		if (!botBeam[beamId])
		{
			botBeam[beamId] = beam = G_Spawn();
			beam->flags			= beamId;
			beam->owner			= self;
			beam->spawnflags	= 1 | 4;
			beam->s.skinnum		= colour;
			beam->enemy			= enemy;
			beam->dmg			= 0;
			beam->classname		= "bfg laser";
			beam->movetype		= MOVETYPE_FLYMISSILE;

			bot_laser_start (beam);
			gi.linkentity (beam);
		}
		else
			bot_Print(self,"Beam aready in use\n");
	}
	else
		bot_Print(self, "Invalid Beam Id\n");
}

void bot_BeamDestroy (int beamId)
{
	if (botBeam[beamId])
	{
		G_FreeEdict(botBeam[beamId]);
		botBeam[beamId] = NULL;
	}
//	bot_Print(NULL, "Beam aready freed\n");
}