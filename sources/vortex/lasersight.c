//This code just handles the laser sight
#include "g_local.h"

void LaserSightThink (edict_t *self);

#define lss ent->lasersight

void lasersight_on (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		end;


	if (!lss)	// Create it
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(end,100 , 0, 0);
		G_ProjectSource (ent->s.origin, end, forward, right, start);
		lss = G_Spawn ();
		lss->owner = ent;
		lss->movetype = MOVETYPE_NOCLIP;
		lss->solid = SOLID_NOT;
		lss->classname = "lasersight";
		lss->s.modelindex = gi.modelindex ("models/sight/tris.md2");
		lss->s.skinnum = 0;   
		lss->s.renderfx |= RF_FULLBRIGHT;
		lss->think = LaserSightThink;  
		lss->nextthink = level.time + 0.1;
	}
}

void lasersight_off (edict_t *ent)
{
	if (lss)	//Remove it
	{
		if (ent->client->oldfov)
			ent->client->ps.fov = ent->client->oldfov;
	   G_FreeEdict(lss);   
	   lss = NULL;
	}
}

void Cmd_LaserSight_f(edict_t *ent)
{
	if (!lss)
		lasersight_on(ent);
	else
		lasersight_off(ent);
}

void Weapon_Railgun (edict_t *ent);
void LaserSightThink (edict_t *self)
{   
	float	distance;
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;
	vec3_t	v;
	trace_t tr;

	AngleVectors (self->owner->client->v_angle, forward, right, up);
	VectorSet(offset,24 , 6, self->owner->viewheight-7);
	G_ProjectSource (self->owner->s.origin, offset, forward, right, start);
	VectorMA(start,8192,forward,end);
	tr = gi.trace (start,NULL,NULL, end,self->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	if (tr.fraction != 1) 
	{     
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);
	}
	if (tr.ent && tr.ent->inuse && tr.ent->takedamage 
		&& tr.ent->solid != SOLID_NOT && tr.ent != self->owner)
	{
		self->s.skinnum = 1;
		
		VectorSubtract(tr.ent->s.origin, self->owner->s.origin, v);
		distance = VectorLength(v);
		// zoom-in on live target with railgun
		/*
		if (tr.ent->health > 0 && distance > 256 && self->owner->client->idle_frames >= 10
			&& self->owner->client->pers.weapon && self->owner->client->pers.weapon->weaponthink == Weapon_Railgun)
		{
			if (!self->owner->client->oldfov)
				self->owner->client->oldfov = self->owner->client->ps.fov;
			VectorSubtract(tr.ent->s.origin, self->owner->s.origin, v);
			distance = VectorLength(v);
			if (distance > 1024.0)
				distance = 1024.0;
			max_zoom = 0.80 * self->owner->client->oldfov;
			zoom = ((float) max_zoom / 1024.0) * distance;
			self->owner->client->ps.fov = self->owner->client->oldfov - zoom;
			self->owner->client->zoomtime = level.time + 2.0;
		}
		*/

	} 
	else  // else this isnt a valid object
	{
		self->s.skinnum = 0;
		// zoom-out
		/*
		if (self->owner->client->oldfov && level.time > self->owner->client->zoomtime)
			self->owner->client->ps.fov = self->owner->client->oldfov;
		*/
	}

	//The following line will make the meatball change angles
	//based on the the meatball is touching
		//vectoangles(tr.plane.normal,self->s.angles);

	//Instead, these line will face the meatball the same 
	//direction as the player
	self->s.angles[PITCH] = self->owner->s.angles[PITCH];
	self->s.angles[YAW] = self->owner->s.angles[YAW];
	self->s.angles[ROLL] = self->owner->s.angles[ROLL];


	VectorCopy(tr.endpos,self->s.origin);   
	gi.linkentity (self);
	self->nextthink = level.time + 0.1;
}
