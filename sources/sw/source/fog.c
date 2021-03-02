#include "g_local.h"

#ifndef AMIGA
#include <windows.h>
#else
#ifdef __PPC__
#include <mgl/gl.h>
#endif
#endif

//entities
void InitTrigger (edict_t *self);

void trigger_fog_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	VectorCopy (self->fog_color, other->fog_color);
	other->fog_density = self->fog_density;

	if(self->spawnflags & 1)
	{
		other->fog_fog = 0;
	}
	else
	{
		other->fog_fog = 1;
	}

	other->fog_changed = true;

//	gi.dprintf("fog touched: %i %i %i\n", other->fog_density, other->fog_fog, other->fog_changed);
}

void Smooth_Fog_Think (edict_t *ent)
{
	ent->owner->fog_changed = true;

	if (ent->owner->fog_color[0] < ent->fog_color[0] - ent->speed)
		ent->owner->fog_color[0] += ent->speed;
	else if (ent->owner->fog_color[0] < ent->fog_color[0])
		ent->owner->fog_color[0] = ent->fog_color[0];
	else if (ent->owner->fog_color[0] > ent->fog_color[0] + ent->speed)
		ent->owner->fog_color[0] -= ent->speed;
	else if (ent->owner->fog_color[0] > ent->fog_color[0])
		ent->owner->fog_color[0] = ent->fog_color[0];


	if (ent->owner->fog_color[1] < ent->fog_color[1] - ent->speed)
		ent->owner->fog_color[1] += ent->speed;
	else if (ent->owner->fog_color[1] < ent->fog_color[1])
		ent->owner->fog_color[1] = ent->fog_color[1];
	else if (ent->owner->fog_color[1] > ent->fog_color[1] + ent->speed)
		ent->owner->fog_color[1] -= ent->speed;
	else if (ent->owner->fog_color[1] > ent->fog_color[1])
		ent->owner->fog_color[1] = ent->fog_color[1];


	if (ent->owner->fog_color[2] < ent->fog_color[2] - ent->speed)
		ent->owner->fog_color[2] += ent->speed;
	else if (ent->owner->fog_color[2] < ent->fog_color[2])
		ent->owner->fog_color[2] = ent->fog_color[2];
	else if (ent->owner->fog_color[2] > ent->fog_color[2] + ent->speed)
		ent->owner->fog_color[2] -= ent->speed;
	else if (ent->owner->fog_color[2] > ent->fog_color[2])
		ent->owner->fog_color[2] = ent->fog_color[2];


	if (ent->owner->fog_density < ent->fog_density - ent->speed)
		ent->owner->fog_density += ent->speed;
	else if (ent->owner->fog_density < ent->fog_density)
		ent->owner->fog_density = ent->fog_density;
	else if (ent->owner->fog_density > ent->fog_density + ent->speed)
		ent->owner->fog_density -= ent->speed;
	else if (ent->owner->fog_density > ent->fog_density)
		ent->owner->fog_density = ent->fog_density;

	//gi.dprintf("smooth fog || owner fog_fog: %f, self fog_fog: %f\n", ent->owner->fog_fog, ent->fog_fog);

	if (ent->fog_color[0] == ent->owner->fog_color[0] && 
		ent->fog_color[1] == ent->owner->fog_color[1] && 
		ent->fog_color[2] == ent->owner->fog_color[2] && 
		ent->fog_density == ent->owner->fog_density)
	{
		G_FreeEdict (ent->owner->fog_ent);
		//gi.dprintf("smooth fog || FREED fog ent\n");
		return;
	}

	ent->nextthink = level.time + FRAMETIME;
}

void trigger_smooth_fog_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//edict_t	*tempent = NULL;

	if (!other->client)
	{
		VectorCopy (self->fog_color, other->fog_color);
		other->fog_density = self->fog_density;
		other->fog_fog = 1;
		return;
	}
	
	if (other->fog_ent)
	{
		G_FreeEdict (other->fog_ent); //remove previous fog temp ent
		//gi.dprintf("smooth fog || FREED old fog ent\n");
	}
	
	other->fog_ent = G_Spawn();

	VectorCopy (self->fog_color, other->fog_ent->fog_color);
	other->fog_ent->fog_density = self->fog_density;

	if(self->spawnflags & 1)
	{
		other->fog_fog = 0;
	}
	else
	{
		other->fog_fog = 1;
	}

	other->fog_ent->speed = self->speed;

	//other->fog_ent = self;
	other->fog_ent->owner = other;

	other->fog_ent->think = Smooth_Fog_Think;
	other->fog_ent->nextthink = level.time + FRAMETIME;

	gi.linkentity (other->fog_ent);
}

/*QUAKED trigger_fog (.5 .5 .5) FOGOFF
	Changes the touching ents fog display.

	"fog_color"		the color of the fog. Should be 3 
					values between 0.0 and 1.0 (for 
					example white is '1.0 1.0 1.0')
	"fog_density"	the density of the fog. (default 1)
	"fog_fog"		how much fog. (default 1)
*/
void SP_trigger_fog (edict_t *self)
{
	if(!FullGL)
	{
		G_FreeEdict(self);
		return;
	}
	
	if (!self->fog_density)
		self->fog_density = 1;
	if (!self->fog_fog)
		self->fog_fog = 1;

//	gi.dprintf("FOG TESTING %f %f %s\n", self->fog_density, self->fog_fog, vtos(self->fog_color));

	InitTrigger (self);
	self->touch = trigger_fog_touch;
}

/*QUAKED trigger_smooth_fog (.5 .5 .5) FOGOFF
Changes the touching ents fog display smoothly.

	"fog_color"		the color of the fog. Should be 3 
					values between 0.0 and 1.0 (for 
					example white is '1.0 1.0 1.0')
	"fog_density"	the density of the fog. (default 1)
					a negative value meens 0.0. (a small
					bug makes 0.0 nothing, so if thats 
					set it will set the value to the 
					default)
	"fog_fog"		how much fog. (default 1)
					a negative value meens 0.0. (a small
					bug makes 0.0 nothing, so if thats 
					set it will set the value to the 
					default)
	"speed"			change in what speed. (default 0.1)
*/

void SP_trigger_smooth_fog (edict_t *ent)
{
	if(!FullGL)
	{
		G_FreeEdict(ent);
		return;
	}
	
	InitTrigger (ent);

	ent->touch = trigger_smooth_fog_touch;

	if (!ent->fog_density < 0)
		ent->fog_density = 0.0;
	else if (!ent->fog_density)
		ent->fog_density = 1.0;

	if (!ent->fog_fog < 0)
		ent->fog_fog = 0.0;
	else if (!ent->fog_fog)
		ent->fog_fog = 1.0;

	if (!ent->speed)
		ent->speed = 0.1;

//	gi.dprintf("smooth fog spawned at %s\n", vtos(ent->s.origin));
//	gi.dprintf("fog_color    %s\n", vtos(ent->fog_color));
//	gi.dprintf("fog_fog      %f\n", ent->fog_fog);
//	gi.dprintf("fog_density  %f\n", ent->fog_density);
//	gi.dprintf("speed        %f\n", ent->speed);
}

void Fog_Update (edict_t *self)
{
	float	fog_color[4];
	int		water;

	return;
	
//	if(!FullGL)
//	{
//		return;
//	}
//
//	if(!fog->value)
//	{
//		glDisable(GL_FOG);
//		return;
//	}

	if(gi.pointcontents(self->client->chasecam->s.origin) & CONTENTS_WATER)
		water = 1;
	else
		water = 0;

#ifdef __PPC__
	if(self->fog_fog == 0 && !water/*!(self->client->ps.rdflags & RDF_UNDERWATER)*/)
	{
		glDisable(GL_FOG);
		return;
	}

	if(self->fog_fog == 0 && water /*(self->client->ps.rdflags & RDF_UNDERWATER)*/)
	{
		fog_color[0] = 0.3f;
		fog_color[1] = 0.3f;
		fog_color[2] = 0.5f;
		fog_color[3] = 1.0f;

		glDisable(GL_FOG);

		glEnable (GL_FOG);
		glFogi (GL_FOG_MODE, GL_LINEAR);
		glHint (GL_FOG_HINT, GL_NICEST);
		glFogfv (GL_FOG_COLOR, (float *)&fog_color);
		glFogf (GL_FOG_START, 1);           // Start the near fog clipping plane to 1
		glFogf (GL_FOG_END, 800);
		return;
	}

	fog_color[0] = self->fog_color[0];
	fog_color[1] = self->fog_color[1];
	fog_color[2] = self->fog_color[2];
	fog_color[3] = 1.0;
	
	//opengl calls
	glDisable(GL_FOG);

	glEnable (GL_FOG);
	glFogi (GL_FOG_MODE, GL_LINEAR);
	glHint (GL_FOG_HINT, GL_NICEST);
//	glFogf (GL_FOG_DENSITY, self->fog_density/100);
	glFogfv (GL_FOG_COLOR, (float *)&fog_color);
	glFogf (GL_FOG_START, 1);
	if(self->fog_density == 0)
		glFogf (GL_FOG_END, 100000);
	else
		glFogf (GL_FOG_END, 10/self->fog_density);
#endif

	self->fog_changed = false;
}
