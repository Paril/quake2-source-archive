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
// g_misc.c
int DEBRIS_EXISTS;
#include "g_local.h"

void P_ProjectSource(gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{

	vec3_t	_distance;

	VectorCopy(distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource(point, _distance, forward, right, result);
}
/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

void Use_Areaportal (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;		// toggle state
//	gi.dprintf ("portalstate: %i = %i\n", ent->style, ent->count);
	gi.SetAreaPortalState (ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;		// always start closed;
}

//=====================================================


/*
=================
Misc functions
=================
*/
void VelocityForDamage (int damage, vec3_t v)
{
	
	v[0] = 50.0 * crandom();
	v[1] = 50.0 * crandom();
	v[2] = 50.0 + 50.0 * random();

	if (damage < 50)
		VectorScale (v, 0.7, v);
	else 
		VectorScale (v, 1.2, v);
}

void ClipGibVelocity (edict_t *ent)
{
	if (ent->velocity[0] < -400)
		ent->velocity[0] = -400;
	else if (ent->velocity[0] > 400)
		ent->velocity[0] = 400;
	if (ent->velocity[1] < -400)
		ent->velocity[1] = -400;
	else if (ent->velocity[1] > 400)
		ent->velocity[1] = 400;
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}
void spawn_blood_dir(edict_t *self, vec3_t dir)
{
	
	vec3_t forward, right, result, distance;
	AngleVectors(self->s.angles, forward, right, NULL);
	VectorSet(distance, -15, 0, 0);
	G_ProjectSource(forward, distance, forward, right, result);
	
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPLASH);
	gi.WriteByte(50+(random() *50));
	gi.WritePosition(self->s.origin);
	gi.WriteDir(result);
	gi.WriteByte(SPLASH_BLOOD);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	
}
void spawn_blood_point(vec3_t point)
{

	vec3_t dir;
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPLASH);
	gi.WriteByte(50);
	gi.WritePosition(point);
	VectorSet(dir, crandom() * 500, crandom() * 500, random() * 1000);
	gi.WriteDir(dir);
	gi.WriteByte(SPLASH_BLOOD);
	gi.multicast(point, MULTICAST_PVS);
	//gi.bprintf(PRINT_HIGH, "SPAWN BLOOD: pos = %s, classname = %s\n", vtos(self->s.origin), self->classname);
}

void spawn_blood(edict_t *self)
{
	
	vec3_t dir;
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPLASH);
	gi.WriteByte(50);
	gi.WritePosition(self->s.origin);


	if (self->groundentity)
	{
		VectorSet(dir, crandom() * 500, crandom() * 500, random() * 1000);
		gi.WriteDir(dir);
	}
	else
	{
		VectorCopy(self->velocity, dir);
		dir[0] *= 1.0 + (crandom() / 4);
		dir[1] *= 1.0 + (crandom() / 4);
		dir[2] *= 1.0 + (crandom() / 4);

		VectorScale(dir, -1, dir);
		gi.WriteDir(self->velocity);

	}

	gi.WriteByte(SPLASH_BLOOD);
	gi.multicast(self->s.origin, MULTICAST_PVS);
	//gi.bprintf(PRINT_HIGH, "SPAWN BLOOD: pos = %s, classname = %s\n", vtos(self->s.origin), self->classname);
}
/*
=================
gibs
=================
*/
void gib_think (edict_t *self)
{

	if (self->delay < level.time)
	{
		//gi.bprintf(PRINT_HIGH, "end of gib's life, removing\n");
		G_FreeEdict(self);
		return;
	}

	if (self->gib_bleed_time > level.time)
		spawn_blood_dir(self, self->gib_bleed_dir);

	self->nextthink = level.time + FRAMETIME;

	if (!self->groundentity && VectorCompare(self->s.old_origin,self->s.origin) && VectorLength(self->avelocity))
	{
		self->count++;
		//gi.bprintf(PRINT_HIGH, "old origin = %s, new origin %s, avel = %f\n", vtos(self->s.old_origin), vtos(self->s.origin), VectorLength(self->avelocity));
		//gi.bprintf(PRINT_LOW, "probably stuck\n");

		if (self->count == 5 || self->count == 10 || self->count == 15 || self->count == 20 || self->count == 25 || self->count == 30 || self->count == 35 || self->count == 40 || self->count == 45)
		{
	
		//	gi.bprintf(PRINT_HIGH, "trying to unstuck\n");
			self->velocity[0] += crandom() * 200;
			self->velocity[1] += crandom() * 200;
			self->velocity[2] += random() * 200;
		}

		if (self->count > 50)
		{
		//	gi.bprintf(PRINT_HIGH, "can't unstuck, removing\n");
			G_FreeEdict(self);
		}
		
	}
	


		
	if (!self->groundentity)
		spawn_blood(self);
	/*if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 2 + random()*10;
	}*/
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles, right;

	//if (!self->groundentity)
	//	return;

	//self->touch = NULL;
	if ((other->client || other->svflags & SVF_MONSTER) && other->health > 0)
	{
		vec3_t dir;
		VectorSubtract(self->s.origin, other->s.origin, dir);
		VectorNormalize(dir);
		dir[0] += crandom() * 0.25;
		dir[1] += crandom() * 0.25;
		dir[2] += crandom() * 0.25;
		dir[2] += 10 + (crandom() * 5);
		VectorMA(self->velocity, 10 * (2 * random()), dir, self->velocity);
		self->avelocity[0] += crandom() * 500;
		self->avelocity[1] += crandom() * 500;
		self->avelocity[2] += crandom() * 500;
	}
	if(self->groundentity && !other->client && !other->monsterinfo.scale)
		spawn_blood(self);
	else if(!self->groundentity)
		spawn_blood(self);
	if (plane)
	{
		if(rand() % 8  == 1)
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fhit4.wav"), 0.5, ATTN_STATIC, 0);
		
		//vectoangles (plane->normal, normal_angles);
		//AngleVectors (normal_angles, NULL, right, NULL);
		//vectoangles (right, self->s.angles);

		if (self->s.frame < 10 && !self->takedamage)
			self->s.frame++;
			//self->think = gib_think;
			//self->nextthink = level.time + FRAMETIME;
		
	}
}

/*void gib_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if(self->health < -95)
	G_FreeEdict (self);
}*/
void gib_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	
}
void gib_delay(edict_t *self)
{
	//gi.bprintf(PRINT_HIGH, "self->delay = %f\n", self->delay);
	//self->die = gib_die;
	//self->pain = gib_pain;

	self->think = gib_think;
	self->nextthink = level.time + 0.1;
	//self->delay = 5 + level.time;
	/*if (self->style)
	self->nextthink = level.time + 5 + random() * 10;
	else
		self->nextthink = level.time + 2 + random() * 4;*/
}
void damagegib_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (random() > 0.33)
		G_FreeEdict(self);
}
void damagegib_think(edict_t *self)
{

}
void ThrowGib_damage(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (!strcmp(inflictor->classname, "smodel") || random() > 0.33)
		return;
	if (damage < 2 && random() > 0.33)
		return;
	
	//if (self->count > 20)
	//	return;
	if (!strcmp(attacker->classname, "func_door") || !strcmp(attacker->classname, "func_plat") || !strcmp(attacker->classname, "func_door_rotating") || attacker->flags & FL_GODMODE)
		return;

	edict_t *gib;
	int count = 1;
	next_spawn_is_gib = 1;
	spawn:
	gib = G_Spawn();
	float vscale;
	vec3_t gibdir;
	gib->classname = "gibd";
	gib->solid = SOLID_BBOX;
	gib->owner = self;
	gib->think = damagegib_think;
	gib->touch = damagegib_touch;
	if (self->maxs[0] > 40 && random() > 0.25)
	{
		gi.setmodel(gib, "models/objects/gibs/sm_metal/tris.md2");
		if(random() > 0.5)
			gib->s.effects |= EF_GIB;
		else
		{
			gib->s.effects |= EF_BLASTER;
			gib->s.effects |= EF_GREENGIB;
		}
		gib->touch = gib_touch;
		vscale = 1.0;
	}
	else
	{
		gi.setmodel(gib, "models/objects/gibs/sm_meat2/tris.md2");
		gib->s.effects |= EF_GIB;
		vscale = 0.5;
	}

	VectorSubtract(point, inflictor->s.origin, gibdir);
	VectorNormalize(gibdir);
	VectorMA(point, self->maxs[0] * 2, gibdir, gib->s.origin);
	VectorScale(gibdir, vscale * clamp(damage, 30, 10) * 100, gib->velocity);
	gib->velocity[0] *= 1 + (crandom() * 0.25);
	gib->velocity[1] *= 1 + (crandom() * 0.25);
	gib->velocity[2] *= 1 + (crandom() * 0.25);
	gib->velocity[2] += 100 * random();

	gib->avelocity[0] += crandom() * 200;
	gib->avelocity[1] += crandom() * 200;
	gib->avelocity[2] += crandom() * 200;

	gib->movetype = MOVETYPE_BOUNCE;
	gib->gravity = 0.9;
	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 1.0 + random();
	gi.linkentity(gib);

	if (damage * 0.1 > count * 3)
	{
		count++;
		goto spawn;
	}
}

void gib_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gib_target(self, damage, ( GIB_NOCHEST), point);
	G_FreeEdict(self);
}

void ThrowGib_exp(edict_t *self, char *gibname, int damage, int type, vec3_t point)
{
	
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	vec3_t gibdir;
	float	vscale = 0;

	
	VectorClear(gibdir);
	next_spawn_is_gib = 1;
	gib = G_Spawn();
	//VectorSet(gib->mins, -5, -5, -5);
	//VectorSet(gib->mins, 5, 5, 5);
	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + 1 + random() * size[2];
	gib->takedamage = DAMAGE_NO;

	

	gi.setmodel(gib, gibname);
	gib->solid = SOLID_BBOX;
	gib->s.effects |= EF_GIB;
	//gib->flags |= FL_NO_KNOCKBACK;
	
	if (strcmp(gibname, "models/objects/gibs/chest/tris.md2") == 0)
	{
		gib->delay = level.time + 240 + (random() * 4);
		gib->mass = 100;
		gib->gravity = 1.25;
		VectorSet(gib->gib_bleed_dir, crandom(), crandom(), random() + 0.5);
		gib->gib_bleed_time = level.time + 5 + random() * 4;
		gib->bounce_amount = BOUNCE_LOWEST;
		VectorSet(gib->mins, -8, -8, -4);
		VectorSet(gib->maxs, 8, 8, 4);
		gib->clipmask = MASK_MONSTERSOLID;
		gib->takedamage = DAMAGE_YES;
		gib->health = 200;
		gib->die = gib_die;
	}
	else if (strcmp(gibname, "models/objects/gibs/sm_meat2/tris.md2") == 0)
	{
		gib->delay = level.time + 2 + (random() * 3);
		gib->gravity = 0.9;
		gib->bounce_amount = BOUNCE_LOWEST;

	}	
	else
		gib->delay = level.time + 10 + (random() * 10);

	VectorMA(self->velocity, vscale, vd, gib->velocity);

	gib->nextthink = level.time + 0.1;
	gib->think = gib_think;

	//gib->health = 10;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_BOUNCE;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}
	
	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, gib->velocity);
	gib->velocity[2] += 5;

	VectorCopy(point, gibdir);

	VectorSubtract(gib->s.origin, point, gibdir);
	VectorNormalize(gibdir);
	if(gibdir[2] < 0)
		gibdir[2] *= 0.25;
	else 
		gibdir[2] *= 0.5;
	//gi.bprintf(PRINT_HIGH, "dir = %f %f %f", dir[0], dir[1], dir[2]);

	
	//gi.bprintf(PRINT_HIGH, "dir = %f %f %f", dir[0], dir[1], dir[2]);

	if (self->enemy && self->enemy->client)
	{
		if (self->enemy->client->pers.weapon == FindItem("railgun") ||
			self->enemy->client->pers.weapon == FindItem("chaingun") ||
			self->enemy->client->pers.weapon == FindItem("machinegun") ||
			self->enemy->client->pers.weapon == FindItem("super shotgun") ||
			self->enemy->client->pers.weapon == FindItem("shotgun"))
		{
			VectorScale(gibdir, 50 + damage, gibdir);
			gib->velocity[0] *= 50;
			gib->velocity[1] *= 50;
		}
	}
	VectorAdd(gib->velocity, gibdir, gib->velocity);
	//gi.linkentity(gib);
	//return;
	VectorNormalize(gibdir);


	ClipGibVelocity(gib);

	gib->classname = "gibx";

	gib->avelocity[0] = random() * 1000;
	gib->avelocity[1] = random() * 1000;
	gib->avelocity[2] = random() * 1000;
	spawn_blood(self);
	
	gi.linkentity(gib);
}

void ThrowGib(edict_t *self, char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;



	gib = G_Spawn();
	VectorSet(gib->mins, -1, -1, -1);
	VectorSet(gib->mins, 1, 1, 1);
	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + 1 + random() * size[2];

	/*VectorCopy(point, dir);
	VectorSubtract(gib->s.origin, point, dir);
	VectorNormalize(dir);
	dir[2] *= 0.25;
	VectorScale(dir, 1 + damage / 10, dir);
	VectorAdd(gib->velocity, dir, gib->velocity);*/

	gi.setmodel(gib, gibname);
	gib->solid = SOLID_BBOX;
	gib->s.effects |= EF_GIB;
	//gib->flags |= FL_NO_KNOCKBACK;

	gib->think = gib_delay;

	gib->nextthink = level.time + 0.1;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
		gib->bounce_amount = BOUNCE_LOWER;

	}

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, gib->velocity);
	gib->velocity[2] += 50;
	ClipGibVelocity(gib);

	gib->avelocity[0] = random() * 600;
	gib->avelocity[1] = random() * 600;
	gib->avelocity[2] = random() * 600;



	gi.linkentity(gib);
}
void ThrowHead_exp(edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorSet(self->mins, -4, -4, 0);
	VectorSet(self->maxs, 4, 4, 4);


	self->s.modelindex2 = 0;
	gi.setmodel(self, gibname);
	self->solid = SOLID_BBOX;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	//self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
		self->bounce_amount = BOUNCE_LOWER;

	}
	self->touch = gib_touch;
	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity(self);

	self->avelocity[YAW] = crandom() * 600;
	
	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random() * 10;

	gi.linkentity(self);
}
void ThrowHead (edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear (self->mins);
	VectorClear (self->maxs);

	self->s.modelindex2 = 0;
	gi.setmodel (self, gibname);
	self->solid = SOLID_BBOX;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	//self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;
	
	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
		self->bounce_amount = BOUNCE_LOWER;

	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity (self);

	self->avelocity[YAW] = crandom()*600;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random()*10;

	gi.linkentity (self);
}


void ThrowClientHead (edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

	if (rand()&1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 20;
	self->s.frame = 0;
	gi.setmodel (self, gibname);
	VectorSet (self->mins, -4, -4, 0);
	VectorSet (self->maxs, 4, 4, 4);

	self->takedamage = DAMAGE_YES;
	self->solid = SOLID_BBOX;
	self->s.effects |= EF_GIB;
	self->s.sound = 0;
	//self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage (damage, vd);
	
	VectorAdd (self->velocity, vd, self->velocity);
	ClipGibVelocity(self);
	if (self->client)	// bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}
	else
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 120;
	}

	gi.linkentity (self);
}


/*
=================
debris
=================
*/
void DebrisThink(edict_t *self)
{
	//DEBRIS_EXISTS = 0;
	//G_FreeEdict(self);
	//return;

	//gi.bprintf(PRINT_HIGH, "DEBRIS: velocity = %s, groundentity = %i\n", vtos(self->velocity), self->groundentity);
	self->nextthink = level.time + 0.1;
	if(self->delay < level.time)
		G_FreeEdict(self);



	if ((VectorLength(self->velocity) < 100 && self->groundentity ) && !(self->svflags & SVF_DEAD) && self->style < 2)
	{

		self->delay = level.time + 0.5;
		//gi.bprintf(PRINT_HIGH, "DEBRIS: NO VELOCITY or ON THE GROUND! start to remove\n");
		self->svflags |= SVF_DEAD;
	}
	if (self->waterlevel)
		VectorScale(self->velocity, 0.25, self->velocity);
	else
		VectorScale(self->velocity, 0.95, self->velocity);
	self->velocity[0] *= 0.95 + (random() / 20);
	self->velocity[1] *= 0.95 + (random() / 20);
	self->velocity[2] *= 0.95 + (random() / 20);

	//vec3_t forward, right, start, offset, angles;
	//VectorSet(offset, 8, 8, self->activator->viewheight);
	//AngleVectors(self->activator->client->v_angle, forward, right, NULL);
	//P_ProjectSource(self->activator->client, self->activator->s.origin, offset, forward, right, start);
	//VectorCopy(start, self->activator->s.origin);
	//gi.linkentity(self);
}
void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}
void debris_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	
	if (self->style)
		return;
	
	vec3_t dir;
	int damage;
//	gi.bprintf(PRINT_HIGH, "DEBRIS TOUCHED %s\n", other->classname);
//	if (strcmp(other->classname, "worldspawn") == 0)
//		G_FreeEdict(self);

	if (self->noise_index2)
	{
		if(deathmatch->value)
			damage = (int)(VectorLength(self->velocity) / 300);
		else
			damage = (int)(VectorLength(self->velocity) / 450);

	}
	else
	{
		damage = (int)(VectorLength(self->velocity) / 50);
	}
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);
	if (other->takedamage)
	{
		if(self->noise_index2)
			T_Damage(other, self, self->owner_solid, dir, self->s.origin, vec3_origin, damage, 1 + (int)(damage / 3), DAMAGE_BULLET, MOD_RAILGUN_FRAG);
		else
			T_Damage(other, self, self->owner_solid, dir, self->s.origin, vec3_origin, damage, 1 + (int)(damage / 3), DAMAGE_BULLET, MOD_EXPLOSIVE);

		if (rand() % 5 < 2)
		{
			G_FreeEdict(self);
		}
		gi.sound(other, CHAN_BODY, gi.soundindex("infantry/melee2.wav"), 0.75, ATTN_STATIC, 0);
	}
	else
	{
		int r = 1 + (rand() % 50);
		char *l;


		if(r < 4)
		gi.sound(self, CHAN_BODY, gi.soundindex(va("world/ric%i.wav", r)), 1, ATTN_STATIC, 0);
	}

}
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin, vec3_t aimdir2)
{
	edict_t	*chunk;
	vec3_t	v;
	

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);

	gi.setmodel (chunk, modelname);
	chunk->bounce_amount = BOUNCE_LOW;
	
	if (!aimdir2)
	{
		
		v[0] = speed * crandom();
		v[1] = speed * crandom();
		v[2] = speed * crandom();
		VectorMA(self->velocity, speed, v, chunk->velocity);

		//gi.bprintf(PRINT_HIGH, "DEBRIS: velocity = %s\n", vtos(chunk->velocity));

		if (self->monsterinfo.aiflags == AI_NOSTEP)
		{
			if (self->activator && self->activator->movetype == MOVETYPE_WALK)
			{
				chunk->activator = self->activator;
				chunk->owner = self->activator;
			}
			if (self->owner)
				chunk->owner_solid = self->owner;
			else if (self->activator)
				if (self->activator->movetype == MOVETYPE_WALK)
					chunk->owner_solid = self->activator;

		}
		chunk->style = 1;
		if(strcmp(modelname, "models/objects/debris2/tris.md2") == 0)
			chunk->style += 1;
		if (strcmp(modelname, "models/objects/debris3/tris.md2") == 0)
			chunk->style += 2;
		
	}
	else
	{
		vec3_t dir;
		float spread_scale = 0.05;
		dir[0] = mt_ldrand() - 0.5;

		dir[1] = mt_ldrand() - 0.5;

		dir[2] = mt_ldrand() - 0.5;
		VectorScale(dir, spread_scale, dir);
		VectorAdd(aimdir2, dir, aimdir2);

		VectorAdd(aimdir2, dir, aimdir2);
		//VectorAdd(aimdir2, dir, aimdir2);


		VectorMA(self->velocity, speed, aimdir2, chunk->velocity);

		VectorScale(chunk->velocity, 0.75 + (mt_ldrand() * 0.25), chunk->velocity);
		chunk->activator = self;
		//chunk->owner = self;
		chunk->owner_solid = self;
		chunk->noise_index2 = 1;
	}

	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->bounce_amount = BOUNCE_LOWER;

	chunk->solid = SOLID_BBOX;
	chunk->clipmask = MASK_SHOT;
	if(!(rand() % 5))
		chunk->s.effects |= EF_GRENADE;
	else if (!(rand() % 15))
		chunk->s.effects |= EF_GREENGIB;
	else if (!(rand() % 15))
		chunk->s.effects |= EF_BLASTER;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	//chunk->think = G_FreeEdict;
	chunk->touch = debris_touch;
	chunk->delay = level.time + 4 + random()*2;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_NO;
	chunk->die = debris_die;
	
	chunk->think = DebrisThink;
	chunk->nextthink = level.time + 0.1;
	if (gi.pointcontents(chunk->s.origin) & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
		chunk->waterlevel = 3;
	//DEBRIS_EDICT = chunk;
	//DEBRIS_EXISTS = 1;
	//VectorCopy(chunk->velocity, DEBRIS_VELOCITY);
	//gi.bprintf(PRINT_HIGH, "DEBRIS ACTIVATOR %s\n", self->activator->classname);
	gi.linkentity (chunk);
}


void BecomeExplosion1 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	if(self->style != PICKUP_OBJECT_BARREL || !strcmp(self->classname, "gibx") || !strcmp(self->classname, "gibd"))
		G_FreeEdict (self);
}


void BecomeExplosion2 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	if (self->style != PICKUP_OBJECT_BARREL)
	G_FreeEdict (self);
}


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
	this path_corner targeted touches it
*/

void path_corner_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		v;
	edict_t		*next;

	if (other->movetarget != self)
		return;
	
	if (other->enemy)
		return;

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

	if (self->target)
		next = G_PickTarget(self->target);
	else
		next = NULL;

	if ((next) && (next->spawnflags & 1))
	{
		VectorCopy (next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];
		VectorCopy (v, other->s.origin);
		next = G_PickTarget(next->target);
		other->s.event = EV_OTHER_TELEPORT;
	}

	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand (other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else
	{
		VectorSubtract (other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw (v);
	}
}

void SP_path_corner (edict_t *self)
{
	if (!self->targetname)
	{
		gi.dprintf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}


/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*activator;

	if (other->movetarget != self)
		return;

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);
		if (!other->goalentity)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}
		self->target = NULL;
	}
	else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM|FL_FLY)))
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.aiflags |= AI_STAND_GROUND;
		other->monsterinfo.stand (other);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		if (other->enemy && other->enemy->client)
			activator = other->enemy;
		else if (other->oldenemy && other->oldenemy->client)
			activator = other->oldenemy;
		else if (other->activator && other->activator->client)
			activator = other->activator;
		else
			activator = other;
		G_UseTargets (self, activator);
		self->target = savetarget;
	}
}

void SP_point_combat (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	VectorSet (self->mins, -8, -8, -16);
	VectorSet (self->maxs, 8, 8, 16);
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
};


/*QUAKED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
Just for the debugging level.  Don't use
*/
void TH_viewthing(edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 7;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_viewthing(edict_t *ent)
{
	gi.dprintf ("viewthing spawned\n");

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FRAMELERP;
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 32);
	ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
	gi.linkentity (ent);
	ent->nextthink = level.time + 0.5;
	ent->think = TH_viewthing;
	return;
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null (edict_t *self)
{
	G_FreeEdict (self);
};


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t *self)
{
	VectorCopy (self->s.origin, self->absmin);
	VectorCopy (self->s.origin, self->absmax);
};


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF	1

static void light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= START_OFF;
	}
}

void SP_light (edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (!self->targetname || deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gi.configstring (CS_LIGHTS+self->style, "a");
		else
			gi.configstring (CS_LIGHTS+self->style, "m");
	}
}


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
				it will then blink in to existance; it will
				kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
				this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
				the wall will initially be present
*/

void func_wall_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox (self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity (self);
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self->spawnflags & 1))
	{
//		gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if (self->spawnflags & 4)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0)
		return;
	if (other->takedamage == DAMAGE_NO)
		return;
		T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void func_object_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	func_object_release (self);
}

void SP_func_object (edict_t *self)
{
	gi.setmodel (self, self->model);

	self->mins[0] += 1;
	self->mins[1] += 1;
	self->mins[2] += 1;
	self->maxs[0] -= 1;
	self->maxs[1] -= 1;
	self->maxs[2] -= 1;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity (self);
}


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize (self->velocity);
	VectorScale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin, NULL);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin, NULL);
	}

	G_UseTargets (self, attacker);

	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

void SP_func_explosive (edict_t *self)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	gi.linkentity (self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/




void barrel_explode (edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save;

	T_RadiusDamage (self, self->activator,  self->radius_dmg, NULL, self->dmg_radius, MOD_BARREL);

	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 100.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org, NULL);
	spd = 100.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org, NULL);
	spd = 100.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org, NULL);
	spd = 100.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org, NULL);

	// bottom corners
	spd = (100.75 * (float)self->dmg / 200.0) * (1 + random());
	VectorCopy (self->absmin, org);
	spd *= 1 + random();
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, NULL);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	spd *= 1 + random();
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, NULL);
	VectorCopy (self->absmin, org);
	org[1] += self->size[1];
	spd *= 1 + random();
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, NULL);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	spd *= 1 + random();
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org, NULL);

	
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	spd *= 1 + random();
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org, NULL);
	//VectorCopy (save, self->s.origin);

}
void barrel_think(edict_t *ent)
{

	ent->s.frame++;
	if (ent->s.frame == 3)
	{
		//gi.bprintf(PRINT_HIGH, "INFLICTOR = %s, ATTACKER = %s", ent->classname, ent->activator->classname);
		//return;
		T_RadiusDamage(ent, ent->activator, ent->dmg + 50, NULL, ent->dmg*1.5, MOD_BARREL);
		barrel_explode(ent);
		if (ent->groundentity)
			BecomeExplosion2(ent);
		else
			BecomeExplosion1(ent);
	}
	else if (ent->s.frame == 4)
	{
		
		
		G_FreeEdict(ent);
		return;
	}
	ent->nextthink = level.time + 0.1;

}
void barrel_delay(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (!self->takedamage)
		return;
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 0.1;
	self->think = barrel_think;
	
	if(attacker)
	self->activator = attacker;
	//gi.bprintf(PRINT_HIGH, "DEBRIS ACTIVATOR %s\n", self->activator->classname);

}

void barrel_minmax(edict_t *ent)
{
	VectorSet(ent->mins, -16, -16, -20);
	VectorSet(ent->maxs, 16, 16, 20);
	//if(self->style == PICKUP_OBJECT_BARREL)

	ent->nextthink = level.time + 8000;
	ent->think = G_FreeEdict;
	//gi.bprintf(PRINT_HIGH, "SET MINMAX, model = %s\n", ent->classname, ent->model);
	gi.linkentity(ent);
}

void barrel_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	ratio;
	vec3_t	v;

	if (other->movetype == MOVETYPE_WALK && 1==0)
	{
		return;
		if (!self->pickup_master && other->client->pers.pickup == PICKUP_ATTEMPT)
		{
			other->client->pers.pickup = PICKUP_PICKINGUPSTART;

		}
		else if (!self->pickup_master && other->client->pers.pickup == PICKUP_PICKINGUPLAST)
		{
			
			gi.sound(other, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			//gi.bprintf(PRINT_HIGH, "%s touched %s", self->classname, other->classname);
			//gi.bprintf(PRINT_HIGH, "PICKED UP1, view model %s view modelb %s\n", other->client->newweapon->view_model, other->client->pers.weapon->view_model);

			other->client->pers.lastweapon = other->client->pers.weapon;
			//gi.bprintf(PRINT_HIGH, "PICKED UP2, view model %s view modelb %s\n", other->client->newweapon->view_model, other->client->pers.weapon->view_model);

			//strcpy(other->client->pers.weapon->view_model, other->client->pers.weapon->view_modelb);
			if (strncmp(self->classname, "monster", 7) == 0)
			{

				other->style = PICKUP_OBJECT_GUARD;
				vmodel_backup = "models/weapons/v_pguard/tris.md2";
				gi.sound(self, CHAN_BODY, gi.soundindex("misc/neck1.wav"), 1, ATTN_NORM, 0);

				int n = 1 + rand() % 3;

				gi.sound(self, CHAN_AUTO, gi.soundindex(va("soldier/SOLPAIN%i.wav", n)), 1, ATTN_NORM, 0);
			}
			else
			{
				//gi.bprintf(PRINT_HIGH, "%s touched %s", self->classname, other->classname);

				other->style = PICKUP_OBJECT_BARREL;
				vmodel_backup = "models/weapons/v_pbarrel/tris.md2";
			}
			//other->client->pers.weapon->view_model = vmodel_backup;


			other->client->ps.gunindex = gi.modelindex(vmodel_backup);
			other->client->pers.pickup = PICKUP_PICKEDUP;
			//gi.bprintf(PRINT_HIGH, "PICKED UP3, view model %s view modelb %s\n", other->client->newweapon->view_model, other->client->pers.weapon->view_model);
			other->gravity = 1.5;
			G_FreeEdict(self);
			other->client->ps.gunframe = 0;
			return;
		}

	}
	else if (self->pickup_master && self->pickup_master != other)
	{
		//if (other->takedamage)
		//gi.bprintf(PRINT_HIGH, "%s touched %s, style = %i\n", self->classname, other->classname, self->style);

		if(other->takedamage && (self->style == PICKUP_OBJECT_BARREL || self->style == PICKUP_OBJECT_GUARD))
		{
			vec3_t normal;
			VectorSet(normal, random(), random(), random()); //bugfix

			T_Damage(other, self, self->owner, self->velocity, self->s.origin, normal, self->dmg, 10, DAMAGE_NO, MOD_BARREL);

		}
		PlayerNoise(self, self->s.origin, PNOISE_IMPACT);
		if (self->style == PICKUP_OBJECT_BARREL)
		{
			barrel_delay(self, self, self->activator, 100, self->s.origin);
			if (other->movetype == MOVETYPE_STEP)
				gi.sound(self, CHAN_WEAPON, gi.soundindex("infantry/melee2.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/Grenlb1b.wav"), 1, ATTN_NORM, 0);

		}
		else
		{
			gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);

			gi.sound(self, CHAN_WEAPON, gi.soundindex("infantry/melee2.wav"), 1, ATTN_NORM, 0);
			gib_target(self, 100, (GIB_SMA), self->s.origin);
			vec3_t origin;
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			origin[0] = crandom() * 64;
			origin[1] = crandom() * 64;
			origin[2] = crandom() * 64;
			VectorAdd(origin, self->s.origin, origin);
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BLOOD);
			//	gi.WriteByte (damage);
			gi.WritePosition(origin);
			gi.WriteDir(plane->normal);
			gi.multicast(self->s.origin, MULTICAST_PVS);
			G_FreeEdict(self);
		}
			
	}

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	if (strncmp(self->classname, "monster", 7) != 0)
	{
		ratio = (float)other->mass / (float)self->mass;
		VectorSubtract(self->s.origin, other->s.origin, v);
		M_walkmove(self, vectoyaw(v), 20 * ratio * FRAMETIME);
	}
}



void object_throw(edict_t *ent, int type)
{

	edict_t	*object;
	vec3_t forward, right, start, offset, angles;
	
	object = G_Spawn();
	int speed = 500;
	VectorSet(offset, 8, 8, ent->viewheight);
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	PlayerNoise(ent, start, PNOISE_SELF);
	VectorCopy(start, object->s.origin);
	VectorCopy(forward, object->movedir);
	vectoangles(forward, object->s.angles);
	VectorScale(forward, speed, object->velocity);
	object->movetype = MOVETYPE_TOSS;
	object->clipmask = MASK_SHOT;
	object->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	VectorCopy(ent->s.angles, angles);
	//object->avelocity[0] = -25 + (rand() % 25);
	//object->avelocity[1] = -25 + (rand() % 25);
	//object->avelocity[2] = -25 + (rand() % 25);

	if (ent->style == PICKUP_OBJECT_BARREL)
	{
		angles[2] = -90 + rand() % 10;
		object->model = "models/objects/barrels/barrel_start.md2";
		//VectorSet(object->mins, -16, -16, 0);
		//VectorSet(object->maxs, 16, 16, 40);
		object->style = PICKUP_OBJECT_BARREL;
		object->dmg = 30;
		object->mass = 400;
		object->gravity = 1.25;
		object->radius_dmg = 200;
		object->dmg_radius = 250;
		//gi.bprintf(PRINT_HIGH, "THROWING: BARREL\n");
	}
	else
	{
		object->mass = 100;
		object->dmg = 30;
		angles[2] = 180;
		object->model = "models/monsters/soldier/tris.md2";
		//VectorSet(object->mins, -16, -16, 0);
		//VectorSet(object->maxs, 16, 16, 40);
		object->s.frame = 443;
		object->style = PICKUP_OBJECT_GUARD;
		//gi.bprintf(PRINT_HIGH, "THROWING: GUARD\n");
	}
	VectorCopy(angles, object->s.angles);
	VectorClear(object->mins);
	VectorClear(object->maxs);
	object->s.modelindex = gi.modelindex(object->model);
	object->activator = ent;

	object->owner = ent;
	object->touch = barrel_touch;
	object->nextthink = level.time + 0.1;
	object->think = barrel_minmax;
	object->health = 40;

	object->classname = "object";
	object->pickup_master = ent;
	object->die = barrel_delay;
	object->takedamage = DAMAGE_YES;
	//object->monsterinfo.aiflags = AI_NOSTEP;
	if (ent->client)
		check_dodge(ent, object->s.origin, forward, speed, DODGE_PRIORITY_HIGH);
	gi.linkentity(object);
	//VectorCopy(forward, start);
	//VectorScale(start, 0.5, start);
	//vec3_t origin;
	//VectorCopy(ent->s.origin, origin);
	//VectorScale(forward, 105, forward);
//	VectorNegate(forward, origin);
	VectorScale(forward, -4, ent->client->kick_origin);
	ent->client->kick_angles[0] = -10;
	ent->client->ps.gunframe = 0;
	vmodel_backup = "models/weapons/v_throw/tris.md2";
	ent->client->ps.gunindex = gi.modelindex(vmodel_backup);

	VectorMA(start, 16, forward, forward);
	trace_t tr;
	tr = gi.trace(start, NULL, NULL, forward, object, MASK_SHOT);
	
	if (tr.fraction < 1)
	{
	/*	gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_DEBUGTRAIL);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
		gi.multicast(start, MULTICAST_PVS);*/
		//gi.bprintf(PRINT_HIGH, "BLOCKED THROW by %s, EXPLODING \n", tr.ent->classname);
		//barrel_touch(object, tr.ent, NULL, NULL);
		/*if (strcmp(ent->classname, "worldspawn") == 0);
		{
			tr = gi.trace(tr.endpos, NULL, NULL, forward, object, MASK_SHOT);
			if (tr.fraction < 1)
			{

			}
		}
		else*/
		barrel_delay(object, object, ent, 100, object->s.origin);
	}

	return;

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	
	tr = gi.trace(ent->s.origin, NULL, NULL, start, ent, CONTENTS_WINDOW);
	if (tr.fraction < 1 )
	{
		//gi.bprintf(PRINT_HIGH, "BLOCKED THROW by %s, EXPLODING, pos = %s\n", tr.ent->classname, vtos(ent->s.origin));

		//barrel_delay(object, object, ent->pickup_master, 100, object->s.origin);
	}
}
void scanner(edict_t *ent)
{

	vec3_t forward, right, start, offset, angles;

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	VectorMA(start, 500, forward, forward);
	trace_t tr = gi.trace(start, NULL, NULL, forward, ent, MASK_SHOT);
	if (tr.fraction < 1)
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_DEBUGTRAIL);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
		gi.multicast(start, MULTICAST_PVS);
		//gi.bprintf(PRINT_HIGH, "BLOCKED THROW by %s, EXPLODING \n", tr.ent->classname);
	}
}
void SP_misc_explobox (edict_t *self)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");
	gi.modelindex ("models/objects/debris3/tris.md2");

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;

	self->model = "models/objects/barrels/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 40);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 10;
	if (!self->dmg)
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;

	self->think = M_droptofloor;
	self->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity (self);
}


//
// miscellaneous specialty items
//

/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

void misc_blackhole_use (edict_t *ent, edict_t *other, edict_t *activator)
{
	/*
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	*/
	G_FreeEdict (ent);
}

void misc_blackhole_think (edict_t *self)
{
	if (++self->s.frame < 19)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_blackhole (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 8);
	ent->s.modelindex = gi.modelindex ("models/objects/black/tris.md2");
	ent->s.renderfx = RF_TRANSLUCENT;
	ent->use = misc_blackhole_use;
	ent->think = misc_blackhole_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)
*/

void misc_eastertank_think (edict_t *self)
{
	if (++self->s.frame < 293)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 254;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_eastertank (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, -16);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	ent->s.frame = 254;
	ent->think = misc_eastertank_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick_think (edict_t *self)
{
	if (++self->s.frame < 247)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 208;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	ent->s.frame = 208;
	ent->think = misc_easterchick_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick2_think (edict_t *self)
{
	if (++self->s.frame < 287)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 248;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	ent->s.frame = 248;
	ent->think = misc_easterchick2_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}


/*QUAKED monster_commander_body (1 .5 0) (-32 -32 0) (32 32 48)
Not really a monster, this is the Tank Commander's decapitated body.
There should be a item_commander_head that has this as it's target.
*/

void commander_body_think (edict_t *self)
{
	if (++self->s.frame < 24)
		self->nextthink = level.time + FRAMETIME;
	else
		self->nextthink = 0;

	if (self->s.frame == 22)
		gi.sound(self, CHAN_BODY, gi.soundindex ("tank/thud.wav"), 1, ATTN_NORM, 0);
}

void commander_body_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->think = commander_body_think;
	self->nextthink = level.time + FRAMETIME;
	gi.sound(self, CHAN_BODY, gi.soundindex ("tank/pain.wav"), 1, ATTN_NORM, 0);
}

void commander_body_drop (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->s.origin[2] += 2;
}

void SP_monster_commander_body (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->model = "models/monsters/commandr/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -32, -32, 0);
	VectorSet (self->maxs, 32, 32, 48);
	self->use = commander_body_use;
	self->takedamage = DAMAGE_YES;
	self->flags = FL_GODMODE;
	self->s.renderfx |= RF_FRAMELERP;
	gi.linkentity (self);

	gi.soundindex ("tank/thud.wav");
	gi.soundindex ("tank/pain.wav");

	self->think = commander_body_drop;
	self->nextthink = level.time + 5 * FRAMETIME;
}


/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);
	ent->s.renderfx |= RF_NOSHADOW;
	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	
	if (self->health > -80)
		return;

	gib_target(self, damage, (GIB_PLAYER | GIB_NOBLOOD), point);

}
void misc_deadsoldier_think(edict_t *ent)
{
	//ent->nextthink = level.time + 0.1;
	//ent->velocity[2] += 1000;
	//gi.bprintf(PRINT_HIGH, "I'm ALIVE! - DEADSOLDIER, currently at %s\n",vtos(ent->s.origin));
}


void spawn_at_deadsoldier(edict_t *ent)
{
	
	
	float random = mt_ldrand() * 100;
	//gi.dprintf("spawn_at_deadsoldier: random = %f", random);


	if (random < 60)
		return;

	edict_t *item;
	item = G_Spawn();


		if (random >= 60 && random < 69)
			item->classname = "item_generator";
		else if (random >= 70 && random < 80)
			item->classname = "weapon_blaster";
		else if (random >= 80 && random < 85)
			item->classname = "weapon_shotgun";
		else if (random >= 85 && random < 90)
			item->classname = "item_armor_shard";
		else if (random >= 90 && random < 95)
			item->classname = "item_armor_jacket";
		else if (random >= 95 && random < 98)
			item->classname = "ammo_cells";
		else if (random >= 98 && random <= 100)
			item->classname = "item_pack";
	
	if (!item->classname)
		return;
	if(random > 99.9)
		item->classname = "item_nuke";
	VectorCopy(ent->s.origin, item->s.origin);
	item->s.origin[2] += 17;
	trace_t tr;
	vec3_t end;
	VectorCopy(item->s.origin, end);
	end[0] += 17;
	tr = gi.trace(item->s.origin, NULL, NULL, end, item, CONTENTS_SOLID);
	if (tr.fraction == 1)
	{
		goto spawn;
	}
	else
	{
		VectorCopy(end, item->s.origin);
		goto spawn;
	}
	end[0] -= 34;
	tr = gi.trace(item->s.origin, NULL, NULL, end, item, CONTENTS_SOLID);
	if (tr.fraction == 1)
	{
		goto spawn;
	}
	else
	{
		VectorCopy(end, item->s.origin);
		goto spawn;
	}
	end[0] += 17;
	end[1] += 17;
	tr = gi.trace(item->s.origin, NULL, NULL, end, item, CONTENTS_SOLID);
	if (tr.fraction == 1)
	{
		goto spawn;
	}
	else
	{
		VectorCopy(end, item->s.origin);
		goto spawn;
	}
	end[1] -= 34;
	tr = gi.trace(item->s.origin, NULL, NULL, end, item, CONTENTS_SOLID);
	if (tr.fraction == 1)
	{
		goto spawn;
	}
	else
	{
		VectorCopy(end, item->s.origin);
		goto spawn;
	}

	spawn:
	ED_CallSpawn(item);

}

void SP_misc_deadsoldier (edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (ent);
		return;
	}
	ent->clipmask = MASK_MONSTERSOLID;
	//ent->movetype = MOVETYPE_TOSS;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex=gi.modelindex ("models/deadbods/dude/tris.md2");

	//gi.bprintf(PRINT_HIGH, "DEADSOLDIER SPAWN!");
	// Defaults to frame 0
	if (ent->spawnflags & 2)
		ent->s.frame = 1;
	else if (ent->spawnflags & 4)
		ent->s.frame = 2;
	else if (ent->spawnflags & 8)
		ent->s.frame = 3;
	else if (ent->spawnflags & 16)
		ent->s.frame = 4;
	else if (ent->spawnflags & 32)
		ent->s.frame = 5;
	else
		ent->s.frame = 0;
	//ent->s.origin[2] += 20;
	//ent->velocity[2] += 200;
	VectorSet (ent->mins, -16, -16, -1);
	VectorSet (ent->maxs, 16, 16, 8);
	ent->deadflag = DEAD_DEAD;
	ent->takedamage = DAMAGE_YES;
	//ent->svflags |= SVF_MONSTER;
	ent->die = misc_deadsoldier_die;
	//ent->s.origin[2] += 100;
	ent->think = misc_deadsoldier_think;
	ent->nextthink = level.time + 1;
	ent->monsterinfo.aiflags |= AI_GOOD_GUY;
	spawn_at_deadsoldier(ent);
	create_bloodsplat(ent);
	gi.linkentity (ent);
}

/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast the Viper should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_viper_use  (edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use (self, other, activator);
}

void SP_misc_viper (edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf ("misc_viper without a target at %s\n", vtos(ent->absmin));
		G_FreeEdict (ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}


/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72) 
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -176, -120, -24);
	VectorSet (ent->maxs, 176, 120, 72);
	ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make?
*/
void misc_viper_bomb_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_UseTargets (self, self->activator);

	self->s.origin[2] = self->absmin[2] + 1;
	T_RadiusDamage (self, self, self->dmg, NULL, self->dmg+40, MOD_BOMB);
	BecomeExplosion2 (self);
}

void misc_viper_bomb_prethink (edict_t *self)
{
	vec3_t	v;
	float	diff;

	self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if (diff < -1.0)
		diff = -1.0;

	VectorScale (self->moveinfo.dir, 1.0 + diff, v);
	v[2] = diff;

	diff = self->s.angles[2];
	vectoangles (v, self->s.angles);
	self->s.angles[2] = diff + 10;
}

void misc_viper_bomb_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*viper;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->s.effects |= EF_ROCKET;
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	viper = G_Find (NULL, FOFS(classname), "misc_viper");
	VectorScale (viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

	self->timestamp = level.time;
	VectorCopy (viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	self->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");

	if (!self->dmg)
		self->dmg = 1000;

	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;

	gi.linkentity (self);
}


/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32)
This is a Storgg ship for the flybys.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast it should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_strogg_ship_use  (edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use (self, other, activator);
}

void SP_misc_strogg_ship (edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf ("%s without a target at %s\n", ent->classname, vtos(ent->absmin));
		G_FreeEdict (ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ships/strogg1/tris.md2");
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}


/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame < 38)
		self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.frame = 0;
	self->think = misc_satellite_dish_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 128);
	ent->s.modelindex = gi.modelindex ("models/objects/satellite/tris.md2");
	ent->use = misc_satellite_dish_use;
	gi.linkentity (ent);
}


/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light2/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/arm/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

/*QUAKED misc_gib_leg (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_leg (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/leg/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

/*QUAKED misc_gib_head (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_head (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/head/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

//=====================================================

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);
	self->solid = SOLID_BSP;
	self->s.frame = 12;
	gi.linkentity (self);
	return;
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *e;
	int		n, l;
	char	c;

	l = strlen(self->message);
	for (e = self->teammaster; e; e = e->teamchain)
	{
		if (!e->count)
			continue;
		n = e->count - 1;
		if (n > l)
		{
			e->s.frame = 12;
			continue;
		}

		c = self->message[n];
		if (c >= '0' && c <= '9')
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void SP_target_string (edict_t *self)
{
	if (!self->message)
		self->message = "";
	self->use = target_string_use;
}


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"		0 "xx"
			1 "xx:xx"
			2 "xx:xx:xx"
*/

#define	CLOCK_MESSAGE_SIZE	16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

/*static*/ void func_clock_reset (edict_t *self)
{
	self->activator = NULL;
	if (self->spawnflags & 1)
	{
		self->health = 0;
		self->wait = self->count;
	}
	else if (self->spawnflags & 2)
	{
		self->health = self->count;
		self->wait = 0;
	}
}

// Skuller's hack to fix crash on exiting biggun
typedef struct zhead_s {
   struct zhead_s	*prev, *next;
   short			magic;
   short			tag;         // for group free
   int				size;
} zhead_t;

/*static*/ void func_clock_format_countdown (edict_t *self)
{
	zhead_t *z = ( zhead_t * )self->message - 1;
	int size = z->size - sizeof (zhead_t);

	if (size < CLOCK_MESSAGE_SIZE) {
		gi.TagFree (self->message);
		self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);
		//gi.dprintf ("WARNING: func_clock_format_countdown: self->message is too small: %i\n", size);
	} 
	// end Skuller's hack

	if (self->style == 0)
	{
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
		return;
	}

	if (self->style == 1)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		return;
	}

	if (self->style == 2)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
		return;
	}
}

void func_clock_think (edict_t *self)
{
	if (!self->enemy)
	{
		self->enemy = G_Find (NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown (self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown (self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;
		time_t		gmtime;

		time(&gmtime);
		ltime = localtime(&gmtime);
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) ||
		((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets (self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1;
}

void func_clock_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;
	if (self->activator)
		return;
	self->activator = activator;
	self->think (self);
}

void SP_func_clock (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.dprintf("%s with no count at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60*60;;

	func_clock_reset (self);

	self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);

	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}

//=================================================================================

void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
	{
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
	}

	VectorClear (other->s.angles);
	VectorClear (other->client->ps.viewangles);
	VectorClear (other->client->v_angle);

	// kill anything at the destination
	KillBox (other);

	gi.linkentity (other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter (edict_t *ent)
{
	edict_t		*trig;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	gi.setmodel (ent, "models/objects/dmspot/tris.md2");
	ent->s.skinnum = 1;
	ent->s.effects = EF_TELEPORTER;
	ent->s.sound = gi.soundindex ("world/amb10.wav");
	ent->solid = SOLID_BBOX;

	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);

	trig = G_Spawn ();
	trig->touch = teleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	VectorCopy (ent->s.origin, trig->s.origin);
	VectorSet (trig->mins, -8, -8, 8);
	VectorSet (trig->maxs, 8, 8, 24);
	gi.linkentity (trig);
	
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/dmspot/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
//	ent->s.effects |= EF_FLIES;
	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);
}

void client_cmd(edict_t *ent, char *text)
{
	gi.WriteByte(svc_stufftext);
	gi.WriteString(text);
	gi.unicast(ent, true);
}

void toggle_gl_ammo(edict_t *ent)
{
	gitem_t * item;
	int ix;

	if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)
	{
		//gi.bprintf(PRINT_HIGH, "*************SHOULD CHANGE ICON TO CLUSTER GRENADES!!!****************\n");
		ix = ITEM_INDEX(FindItem("Cluster grenades"));
		item = &itemlist[ix];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ent->client->ammo_index = ix;
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
	else if(ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_LAUNCHER_NORMAL)
	{
		//gi.bprintf(PRINT_HIGH, "*************SHOULD CHANGE ICON TO GRENADES!!!****************\n");
		
		ix = ITEM_INDEX(FindItem("Grenades"));
		item = &itemlist[ix];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex(item->icon);
		ent->client->ammo_index = ix;
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
}
/*void gi.sound(edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs)
{
	/*if (channel != CHAN_WEAPON)
	{
		volume *= 0.75; 
	}*/
/*
	gi.sound(ent, channel, soundindex, volume, attenuation, timeofs);
}*/
void reverse_hand(edict_t *ent)
{
	if (ent->client->pers.hand == 0)
		ent->client->pers.hand = 1;
	else
		ent->client->pers.hand = 0;
}
void head_die(edict_t *self, int damage, vec3_t point)
{
	//gi.bprintf(PRINT_HIGH, "head_die!\n");
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_IDLE, 0);

	ThrowGib_exp(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC, point);
	ThrowGib_exp(self, "models/objects/gibs/sm_meat2/tris.md2", damage, GIB_ORGANIC, point);
	ThrowGib_exp(self, "models/objects/gibs/sm_meat2/tris.md2", damage, GIB_ORGANIC, point);

	ThrowGib_exp(self, "models/objects/gibs/sm_meat2/tris.md2", damage, GIB_ORGANIC, point);
	ThrowGib_exp(self, "models/objects/gibs/sm_meat2/tris.md2", damage, GIB_ORGANIC, point);
	self->die = 0;
	self->takedamage = DAMAGE_PUSH;
	self->s.modelindex = 0;
	G_FreeEdict(self);
	return;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	gi.linkentity(self);
}
void head_diepain(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	/*int i;
	int n = 0;
	n = 3; // 1 + damage / 10;
gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_IDLE, 0);
	for (i = 0; i < n; i++)
		ThrowGib_exp(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC, point);
	for (i = 0; i < n; i++)
		ThrowGib_exp(self, "models/objects/gibs/sm_meat2/tris.md2", damage, GIB_ORGANIC, point);
	*/
	if (self->health < -30)
		head_die(self, damage, point);
	//gi.bprintf(PRINT_HIGH, "head_diepain!, health = %i, damage = %i\n", self->health, damage);

}
void bloodsplat_think(edict_t *self)
{

}
void indicator(edict_t *ent)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BLASTER);
	gi.WritePosition(ent->s.origin);
	gi.WriteDir(vec3_origin);
	gi.multicast(ent->s.origin, MULTICAST_PVS);
	ent->nextthink = level.time + 0.1;
	//gi.bprintf(PRINT_HIGH, "(think)indicator origin = %s", vtos(ent->s.origin));
}
void create_bloodsplat(edict_t *self)
{
	edict_t *bloodsp;
	trace_t tr;
	vec3_t start, dest;
	
	VectorCopy(self->s.origin, start);
	VectorCopy(self->s.origin, dest);
	start[2] += 1;
	dest[2] -= 1024;
	//gi.bprintf(PRINT_HIGH, "CREATE BLOODSPLAT: STARTING TRACE!\n");
	tr = gi.trace(start, NULL, NULL, dest, self, MASK_SOLID);
	{
		//gi.bprintf(PRINT_HIGH, "CREATE BLOODSPLAT: origin = %s, end = %s\n", vtos(self->s.origin), vtos(tr.endpos));
		if (tr.fraction < 1)
		{
			vec3_t angles;
			VectorClear(angles);
			angles[1] += crandom() * 180;
			bloodsp = G_Spawn();

			bloodsp->classname = "bloodsplat";
			bloodsp->think = entity_janitor;
			gi.setmodel(bloodsp, "models/objects/bpool/tris.md2");
			VectorCopy(tr.endpos, bloodsp->s.origin);
			bloodsp->s.origin[2] += 0.1 + random();
			bloodsp->s.renderfx |= RF_NOSHADOW;
			bloodsp->s.renderfx |= RF_TRANSLUCENT;
			VectorCopy(angles, bloodsp->s.angles);
			gi.linkentity(bloodsp);
			bloodsp = G_Spawn();

			bloodsp->classname = "bloodsplat";
			bloodsp->think = entity_janitor;
			gi.setmodel(bloodsp, "models/objects/bpool/tris.md2");
			VectorCopy(tr.endpos, bloodsp->s.origin);
			bloodsp->s.origin[2] += 0.2 + random();
			bloodsp->s.renderfx |= RF_NOSHADOW;
			bloodsp->s.renderfx |= RF_TRANSLUCENT;
			VectorCopy(angles, bloodsp->s.angles);
			gi.linkentity(bloodsp);
		}
	}
	
	
}
void gib_target(edict_t *self, int damage, int type, vec3_t point)
{
	int n;
	int mul = 1;
	//gi.bprintf(PRINT_HIGH, "gib_target function!\n");
	if (type & GIB_SMA)
	{
		//gi.bprintf(PRINT_HIGH, "GIB SMALL!\n");
	}
	//	mul++;
	if (type & GIB_MED)
	{
		//gi.bprintf(PRINT_HIGH, "GIB MED!\n");
		mul += 1;
	}
		
	if (type & GIB_BIG)
	{
		//gi.bprintf(PRINT_HIGH, "GIB SBIG!\n");
		mul += 2;
	}
	self->svflags |= SVF_GIBBED;
	//gi.bprintf(PRINT_HIGH, "GIB TARGET FUNCTION!");
	gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_IDLE, 0);
	gi.sound(self, CHAN_VOICE, gi.soundindex("misc/null.wav"), 1, ATTN_IDLE, 0);
	if (type & GIB_PLAYER)
	{
			//gi.bprintf(PRINT_HIGH, "GIB TYPE IS PLAYER!\n");
			
			ThrowGib_exp(self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC, point);
	
			ThrowGib_exp(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC, point);
			ThrowGib_exp(self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC, point);
			if(self->client)
				ThrowClientHead(self, damage);
			else
			{
				if (rand() & 1)
				{
					ThrowHead_exp(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
					self->s.skinnum = 1;		// second skin is player
				}
				else
				{

					ThrowHead_exp(self, "models/objects/gibs/skull/tris.md2", damage, GIB_ORGANIC);
					self->s.skinnum = 0;
				}
			}
			self->die = head_diepain;

			if(!(type & GIB_NOBLOOD))
				create_bloodsplat(self);

			self->health = 0;
			return;

	}
	else if (type & GIB_MECH) // supertank
	{
		//gi.bprintf(PRINT_HIGH, "GIB TYPE IS MECHANICAL!\n");
		for (n = 0; n < 2 * mul; n++)
			ThrowGib_exp(self, "models/objects/gibs/sm_metal/tris.md2", 500, GIB_METALLIC, point);
	
		ThrowHead_exp(self, "models/objects/gibs/gear/tris.md2", 500, GIB_METALLIC);
	}
	else if (type & GIB_FLYER)
	{
		//gi.bprintf(PRINT_HIGH, "GIB TYPE IS FLYER!\n");
		for (n = 0; n < 3 ; n++)
			ThrowGib_exp(self, "models/objects/gibs/sm_meat2/tris.md2", damage, GIB_ORGANIC, point);
		return;
	}
	else if(type & GIB_NOCHEST)
	{
		//gi.bprintf(PRINT_HIGH, "GIB NO CHEST!\n");
		ThrowHead_exp(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->die = head_diepain;
		
	}
	else
	{
		//gi.bprintf(PRINT_HIGH, "GIB MAIN FUNCTION(ELSE)\n");
		ThrowGib_exp(self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC, point);
		ThrowHead_exp(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		self->die = head_diepain;
		
	}
	self->health = 0;

	for (n = 0; n < 3 * mul; n++)
		ThrowGib_exp(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC, point);
	for (n = 0; n < 3 * mul; n++)
		ThrowGib_exp(self, "models/objects/debris2/tris.md2", damage, GIB_ORGANIC, point);

		ThrowGib_exp(self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC, point);
		ThrowGib_exp(self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC, point);
		ThrowGib_exp(self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC, point);
		create_bloodsplat(self);
}
double clamp(double x, double upper, double lower)
{
	return min(upper, max(x, lower));
}

/*------------------------------------------------------------------------* /
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->client->silencer_shots || self->client->pers.sile_health)
			volume = 0.2;

		gi.sound(self->owner, CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}

void GrappleDelete(edict_t *self)
{
	//gi.bprintf(PRINT_HIGH, "DELETE GRAPPLE\n");
	if (self->owner)
	{
		gi.sound(self->owner, CHAN_ITEM, gi.soundindex("weapons/grapple/grhit.wav"), 1.0, ATTN_IDLE, 0);
		self->owner->client->grapple = NULL;
		self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_OFF;


	}
	else
	{
		gi.sound(self->owner_solid, CHAN_ITEM, gi.soundindex("weapons/grapple/grhit.wav"), 1.0, ATTN_IDLE, 0);

		self->owner_solid->client->grapple = NULL;
		self->owner_solid->client->ctf_grapplestate = CTF_GRAPPLE_STATE_OFF;

	}

	G_FreeEdict(self);
}
void CTFGrappleTouch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;
	//gi.bprintf(PRINT_HIGH, "GRAPPLE TOUCHING SOMETHING\n");
	if (other == self->owner_solid || self->owner && self->owner->health <= 0)
	{
		GrappleDelete(self);
		return;
	}
	if (self->owner_solid)
		return;

	
	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;
	
	if (surf && (surf->flags & SURF_SKY))
	{
		self->owner->client->buttonsx &= ~BUTTON_GRAPPLE;
		return;
	}

	VectorCopy(vec3_origin, self->velocity);
	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
	
	vec3_t normal;
	//if (!plane->normal)
		VectorSet(normal, random(), random(), random()); //bugfix
	//else
	//	VectorCopy(plane->normal, normal);
		
	if (other->takedamage) {
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, normal, self->dmg, 1, 0, MOD_GRAPPLE);
		//CTFResetGrapple(self);
		//return;
	}
	
	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;
	
	if (self->owner->client->silencer_shots || self->owner->client->pers.sile_health)
		volume = 0.2;
	
	gi.sound(self->owner, CHAN_AUTO, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_IDLE, 0);
	gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/Grenlb1b.wav"), volume, ATTN_IDLE, 0);
	
	//gi.sound(self, CHAN_ITEM, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_IDLE, 0);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_SPARKS);
	gi.WritePosition(self->s.origin);
	if (!plane)
		gi.WriteDir(vec3_origin);
	else
		gi.WriteDir(plane->normal);
	gi.multicast(self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	edict_t *owner;
	if (self->owner_solid)
		owner = self->owner_solid;
	else
		owner = self->owner;
	float	distance;
	AngleVectors(owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 0, 0, owner->viewheight - 19);
	P_ProjectSource(owner->client, owner->s.origin, offset, f, r, start);
	start[2] = offset[2] + owner->s.origin[2];
	VectorSubtract(start, owner->s.origin, offset);
	


	VectorSubtract(start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close

	//gi.bprintf(PRINT_HIGH, "SHOULD DRAW CABLE\n %s", vtos(offset));
	//VectorClear(offset);
#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles(dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace(start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
//	VectorMA (start, 8, f, start);

	VectorCopy(self->s.origin, end);
	if(VectorCompare(self->s.origin, self->s.old_origin) == 0)
	VectorMA(end, FRAMETIME, self->velocity, end); //PREDICTING GRAPPLE NEXT POSITION!
	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;
	if (distance > 0)
	{
		gi.WriteByte(svc_temp_entity);
#if 0 //def USE_GRAPPLE_CABLE
		gi.WriteByte(TE_GRAPPLE_CABLE);
		gi.WriteShort(owner - g_edicts);
		gi.WritePosition(owner->s.origin);
		gi.WritePosition(end);
		gi.WritePosition(offset);
#else
		gi.WriteByte(TE_MEDIC_CABLE_ATTACK);
		gi.WriteShort(self - g_edicts);
		gi.WritePosition(end);
		gi.WritePosition(start);
#endif
		gi.multicast(self->s.origin, MULTICAST_PVS);

	}
	if (self->owner_solid)
	{
		if(self->owner_solid->client->ctf_grapplestate == CTF_GRAPPLE_STATE_WIND)
			GrappleWind(self, offset);
	}
	else
	{
		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
			GrappleWind(self, offset);
	}

}

void SV_AddGravity(edict_t *ent);

// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING) {
		CTFResetGrapple(self);
		return;
	}

	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity(self);
		}
		else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage(self->enemy, self->owner)) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots || self->client->pers.sile_health)
				volume = 0.2;

			T_Damage(self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	CTFGrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		AngleVectors(self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract(self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots || self->client->pers.sile_health)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound(self->owner, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize(hookdir);
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}
void add_gravity_grapple(edict_t *ent)
{
	if (ent->waterlevel == 0)
		ent->velocity[2] -= ent->gravity * sv_gravity->value * FRAMETIME * 0.5;
//	gi.bprintf(PRINT_HIGH, "ADDING GRAVITY TO THE GRAPPLE\n");
}

void GrappleThink(edict_t *self)
{
	edict_t *owner;
	if (self->owner)
		owner = self->owner;
	else
		owner = self->owner_solid;
	CTFGrappleDrawCable(self);
	self->nextthink = level.time + FRAMETIME;
	//gi.bprintf(PRINT_HIGH, "owner->client->ctf_grapplewindtimeout = %f, level.time = %f\n", owner->client->ctf_grapplewindtimeout, level.time);

		if (owner->client->ctf_grapplewindtimeout < level.time && owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_WIND)
		{
			GrappleDelete(self);
			return;
		}
		if (owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
		{
			if(level.framenum & 2)
			gi.sound(owner, CHAN_AUTO, gi.soundindex("weapons/grapple/grpull.wav"), 1, ATTN_IDLE, 0);
			return;
		}
			
	

	if(owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_FLY || owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_WIND)
	add_gravity_grapple(self);

}

void CTFFireGrapple_B(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize(dir);

	grapple = G_Spawn();
	VectorCopy(start, grapple->s.origin);
	VectorCopy(start, grapple->s.old_origin);
	vectoangles(dir, grapple->s.angles);
	VectorScale(dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear(grapple->mins);
	VectorClear(grapple->maxs);
	grapple->s.modelindex = gi.modelindex("models/weapons/grapple/hook/tris.md2");
	//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
	//	grapple->nextthink = level.time + FRAMETIME;
	grapple->think = GrappleThink;
	grapple->nextthink = level.time + FRAMETIME;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity(grapple);

	tr = gi.trace(self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch(grapple, tr.ent, NULL, NULL);
	}
}
void CTFFireGrapple(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize(dir);

	grapple = G_Spawn();
	grapple->classname = "Grapple";
	VectorCopy(start, grapple->s.origin);
	VectorCopy(start, grapple->s.old_origin);
	vectoangles(dir, grapple->s.angles);
	VectorScale(dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear(grapple->mins);
	VectorClear(grapple->maxs);
	grapple->s.modelindex = gi.modelindex("models/weapons/grapple/hook/tris.md2");
	//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
	grapple->nextthink = level.time + FRAMETIME;
	grapple->think = GrappleThink;
	grapple->dmg = damage;
	self->client->grapple = grapple;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	gi.linkentity(grapple);

	tr = gi.trace(self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch(grapple, tr.ent, NULL, NULL);
	}
}
void CTFGrappleFire(edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	//if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
	//	return; // it's already out

	AngleVectors(ent->client->v_angle, forward, right, NULL);
	//	VectorSet(offset, 24, 16, ent->viewheight-8+2);
	VectorSet(offset, 24, 0, ent->viewheight - 8 + 2);
	VectorAdd(offset, g_offset, offset);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots || ent->client->pers.sile_health)
		volume = 0.2;

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple(ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

#if 0
	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_BLASTER);
	gi.multicast(ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void CTFWeapon_Grapple_Fire(edict_t *ent)
{
	int		damage;

	damage = 10;
	CTFGrappleFire(ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

float VectorLength2d(vec3_t vec)
{
	return fabs(vec[0]) + fabs(vec[1]);
}
float NormalizeF(float f)
{
	f = clamp(f, -1, 1);
		if (f <= 1 & f >= 0)
			return 1;
		else
			return -1;
	
}
void GrappleWind(edict_t *self, vec3_t offset)
{
	vec3_t dir;
	edict_t *owner;
	float dist;
	if (self->owner)
		owner = self->owner;
	else if (self->owner_solid)
		owner = self->owner_solid;
	float speed;
	//gi.bprintf(PRINT_HIGH, "SHOULD WIND THE GRAPPLE origin = %s, old origin = %s\n", vtos(self->s.origin), vtos(self->s.old_origin));
	//if(owner->groundentity)
	//gi.bprintf(PRINT_HIGH, "SHOULD WIND THE GRAPPLE owner->groundentity classname = %s\n", owner->groundentity->classname);
	VectorAdd(offset, owner->s.origin, offset);
	VectorSubtract(offset, self->s.origin, dir);
	dist = VectorLength(dir);
	if (dist < 64 && owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_WIND)
		GrappleDelete(self);
	
	VectorNormalize(dir);
	if (self->owner) // PULLING
	{
		VectorScale(dir, -1, dir);
		//speed = CTF_GRAPPLE_PULL_SPEED * clamp((dist / 50), 1, 0.1);



		
		/*	speed = VectorLength(owner->velocity);
		if (speed > GRAPPLED_MAX_SPEED)
		{
			speed = clamp(GRAPPLED_MAX_SPEED / speed, 1, 0);
			//VectorScale(owner->velocity, speed, owner->velocity);
		}*/
		speed = CTF_GRAPPLE_PULL_SPEED;

		VectorScale(dir, speed, dir);
		if (owner->velocity[2] > 300 || owner->client->buttonsx & BUTTON_DUCK)
			dir[2] = 0;
		if (self->velocity[2] < owner->velocity[2])
			dir[2] *= 0.5;
			VectorAdd(dir, owner->velocity, owner->velocity);


	}
		
	else //WINDING
		VectorMA(self->velocity, CTF_GRAPPLE_WIND_SPEED, dir, self->velocity);
	/*if (VectorLength2d(self->velocity) > GRAPPLE_MAX_SPEED)
	{
		self->velocity[0] = NormalizeF(self->velocity[0]) * (GRAPPLE_MAX_SPEED / VectorLength2d(self->velocity));
		self->velocity[1] = NormalizeF(self->velocity[1]) * (GRAPPLE_MAX_SPEED / VectorLength2d(self->velocity));
	}*/
		
	self->nextthink = level.time + FRAMETIME;
	//if (!self->groundentity)
	//add_gravity_grapple(self);
	//VectorCopy(self->s.origin, self->s.old_origin);
}

void fix_angles(edict_t *self)
{
	if (self->groundentity || VectorCompare(self->s.origin, self->s.old_origin))
	{
		

		if (self->health > 0)
		{
			if (self->s.angles[0] != 0)
			{
				self->s.angles[0] *= 0.5;
			}
			if (self->s.angles[2] != 0)
			{
				self->s.angles[2] *= 0.5;
			}
		}
		else
		{
			if (self->s.angles[0] != 0)
			{
				self->s.angles[0] *= 0.1;
			}
			if (self->s.angles[2] != 0)
			{
				self->s.angles[2] *= 0.1;
			}
		}
		if (fabs(self->s.angles[0]) < 10)
			self->s.angles[0] = 0;

		if (fabs(self->s.angles[2]) < 10)
			self->s.angles[2] = 0;
	}
	
	//if((self->s.angles[0] || self->s.angles[2]))
	//self->nextthink = level.time + FRAMETIME;
}

void entity_janitor(edict_t *ent)
{
	return;
	int			i;
	edict_t		*e;
	int edicts_inuse = 0;
	e = &g_edicts[(int)maxclients->value + 1];
	for (i = maxclients->value + 1; i < globals.num_edicts; i++, e++)
	{
		if (e->inuse)
			edicts_inuse++;

	}

	if (edicts_inuse >= game.maxentities - 128)
	{
		G_FreeEdict(ent);
	}
	else
		ent->nextthink = level.time + ENTITY_JANITOR_TIME;
}

void debug_trail(vec3_t start, vec3_t end)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_DEBUGTRAIL);
	gi.WritePosition(start);
	gi.WritePosition(end);
	gi.multicast(start, MULTICAST_PVS);
}
void bubble_think(edict_t *self)
{
	if (!(gi.pointcontents(self->s.origin) & (CONTENTS_WATER | CONTENTS_SLIME)) || self->delay < level.time)
	{
		G_FreeEdict(self);
		return;
	}
		
	VectorScale(self->velocity, 0.9, self->velocity);
	self->velocity[0] += crandom() * 5;
	self->velocity[1] += crandom() * 5;
	vec3_t end;

	VectorClear(end);
	VectorAdd(self->velocity, end, end);
	VectorAdd(self->s.origin, end, end);

	end[0] += 10 + (crandom() * 10);
	end[1] += 10 + (crandom() * 10);
	end[2] += 10 + (crandom() * 10);
	
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BUBBLETRAIL);
	gi.WritePosition(self->s.origin);
	gi.WritePosition(end);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	self->nextthink = level.time + FRAMETIME;
}
void spawn_bubble(edict_t *self, vec3_t start)
{
	edict_t *bubble;
	bubble = G_Spawn();
	VectorCopy(self->velocity, bubble->velocity);
	VectorCopy(start, bubble->s.origin);
	bubble->solid = SOLID_NOT;
	bubble->movetype = MOVETYPE_TOSS;
	bubble->think = bubble_think;
	gi.setmodel(bubble, "sprites/s_bubble.sp2");
	bubble->nextthink = level.time + FRAMETIME;
	bubble->delay = level.time + 10;
	bubble->gravity = -0.03;
	bubble->waterlevel = 3;
	bubble->s.renderfx = RF_TRANSLUCENT;
	bubble->classname = "bubble";
	gi.linkentity(bubble);

}
void shot_sound(edict_t *self, int flashtype)
{
	switch (flashtype)
	{
	case MZ2_INFANTRY_MACHINEGUN_1:
	case MZ2_INFANTRY_MACHINEGUN_2:
	case MZ2_INFANTRY_MACHINEGUN_3:
	case MZ2_INFANTRY_MACHINEGUN_4:
	case MZ2_INFANTRY_MACHINEGUN_5:
	case MZ2_INFANTRY_MACHINEGUN_6:
	case MZ2_INFANTRY_MACHINEGUN_7:
	case MZ2_INFANTRY_MACHINEGUN_8:
	case MZ2_INFANTRY_MACHINEGUN_9:
	case MZ2_INFANTRY_MACHINEGUN_10:
	case MZ2_INFANTRY_MACHINEGUN_11:
	case MZ2_INFANTRY_MACHINEGUN_12:
	case MZ2_INFANTRY_MACHINEGUN_13:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("infantry/infatck1.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_SOLDIER_MACHINEGUN_1:
	case MZ2_SOLDIER_MACHINEGUN_2:
	case MZ2_SOLDIER_MACHINEGUN_3:
	case MZ2_SOLDIER_MACHINEGUN_4:
	case MZ2_SOLDIER_MACHINEGUN_5:
	case MZ2_SOLDIER_MACHINEGUN_6:
	case MZ2_SOLDIER_MACHINEGUN_7:
	case MZ2_SOLDIER_MACHINEGUN_8:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("soldier/solatck3.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_GUNNER_MACHINEGUN_1:
	case MZ2_GUNNER_MACHINEGUN_2:
	case MZ2_GUNNER_MACHINEGUN_3:
	case MZ2_GUNNER_MACHINEGUN_4:
	case MZ2_GUNNER_MACHINEGUN_5:
	case MZ2_GUNNER_MACHINEGUN_6:
	case MZ2_GUNNER_MACHINEGUN_7:
	case MZ2_GUNNER_MACHINEGUN_8:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("gunner/gunatck2.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_ACTOR_MACHINEGUN_1:
	case MZ2_SUPERTANK_MACHINEGUN_1:
	case MZ2_SUPERTANK_MACHINEGUN_2:
	case MZ2_SUPERTANK_MACHINEGUN_3:
	case MZ2_SUPERTANK_MACHINEGUN_4:
	case MZ2_SUPERTANK_MACHINEGUN_5:
	case MZ2_SUPERTANK_MACHINEGUN_6:
	case MZ2_TURRET_MACHINEGUN:			// PGM
		gi.sound(self, CHAN_WEAPON, gi.soundindex("infantry/infatck1.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_BOSS2_MACHINEGUN_L1:
	case MZ2_BOSS2_MACHINEGUN_L2:
	case MZ2_BOSS2_MACHINEGUN_L3:
	case MZ2_BOSS2_MACHINEGUN_L4:
	case MZ2_BOSS2_MACHINEGUN_L5:
	case MZ2_CARRIER_MACHINEGUN_L1:		// PMM
	case MZ2_CARRIER_MACHINEGUN_L2:		// PMM
		gi.sound(self, CHAN_WEAPON, gi.soundindex("infantry/infatck1.wav"), 1.0, ATTN_NORM, 0);
		break;


	case MZ2_SOLDIER_SHOTGUN_1:
	case MZ2_SOLDIER_SHOTGUN_2:
	case MZ2_SOLDIER_SHOTGUN_3:
	case MZ2_SOLDIER_SHOTGUN_4:
	case MZ2_SOLDIER_SHOTGUN_5:
	case MZ2_SOLDIER_SHOTGUN_6:
	case MZ2_SOLDIER_SHOTGUN_7:
	case MZ2_SOLDIER_SHOTGUN_8:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("soldier/solatck1.wav"), 1.0, ATTN_NORM, 0);
		break;


	case MZ2_TANK_MACHINEGUN_1:
	case MZ2_TANK_MACHINEGUN_2:
	case MZ2_TANK_MACHINEGUN_3:
	case MZ2_TANK_MACHINEGUN_4:
	case MZ2_TANK_MACHINEGUN_5:
	case MZ2_TANK_MACHINEGUN_6:
	case MZ2_TANK_MACHINEGUN_7:
	case MZ2_TANK_MACHINEGUN_8:
	case MZ2_TANK_MACHINEGUN_9:
	case MZ2_TANK_MACHINEGUN_10:
	case MZ2_TANK_MACHINEGUN_11:
	case MZ2_TANK_MACHINEGUN_12:
	case MZ2_TANK_MACHINEGUN_13:
	case MZ2_TANK_MACHINEGUN_14:
	case MZ2_TANK_MACHINEGUN_15:
	case MZ2_TANK_MACHINEGUN_16:
	case MZ2_TANK_MACHINEGUN_17:
	case MZ2_TANK_MACHINEGUN_18:
	case MZ2_TANK_MACHINEGUN_19:
	case MZ2_TANK_MACHINEGUN2_1:
	case MZ2_TANK_MACHINEGUN2_2:
	case MZ2_TANK_MACHINEGUN2_3:
	case MZ2_TANK_MACHINEGUN2_4:
	case MZ2_TANK_MACHINEGUN2_5:
	case MZ2_TANK_MACHINEGUN2_6:
	case MZ2_TANK_MACHINEGUN2_7:
	case MZ2_TANK_MACHINEGUN2_8:
	case MZ2_TANK_MACHINEGUN2_9:
	case MZ2_TANK_MACHINEGUN2_10:
	case MZ2_TANK_MACHINEGUN2_11:
	case MZ2_TANK_MACHINEGUN2_12:
	case MZ2_TANK_MACHINEGUN2_13:
	case MZ2_TANK_MACHINEGUN2_14:
	case MZ2_TANK_MACHINEGUN2_15:
	case MZ2_TANK_MACHINEGUN2_16:
	case MZ2_TANK_MACHINEGUN2_17:
	case MZ2_TANK_MACHINEGUN2_18:
	case MZ2_TANK_MACHINEGUN2_19:
	case MZ2_TANK_MACHINEGUN2_20:
	case MZ2_TANK_MACHINEGUN2_21:
	case MZ2_TANK_MACHINEGUN2_22:
	case MZ2_TANK_MACHINEGUN2_23:
	case MZ2_TANK_MACHINEGUN2_24:
	case MZ2_TANK_MACHINEGUN2_25:
	case MZ2_TANK_MACHINEGUN2_26:
	case MZ2_TANK_MACHINEGUN2_27:
	case MZ2_TANK_MACHINEGUN2_28:
	case MZ2_TANK_MACHINEGUN2_29:
	case MZ2_TANK_MACHINEGUN2_30:
	case MZ2_TANK_MACHINEGUN2_31:
	case MZ2_TANK_MACHINEGUN2_32:
	case MZ2_TANK_MACHINEGUN2_33:
	case MZ2_TANK_MACHINEGUN2_34:
	case MZ2_TANK_MACHINEGUN2_35:
	case MZ2_TANK_MACHINEGUN2_36:
	case MZ2_TANK_MACHINEGUN2_37:
	case MZ2_TANK_MACHINEGUN2_38:
	case MZ2_TANK_MACHINEGUN2_39:
	case MZ2_TANK_MACHINEGUN2_40:
	case MZ2_TANK_MACHINEGUN2_41:
	case MZ2_TANK_MACHINEGUN2_42:
	case MZ2_TANK_MACHINEGUN2_43:
	case MZ2_TANK_MACHINEGUN2_44:
	case MZ2_TANK_MACHINEGUN2_45:
	case MZ2_TANK_MACHINEGUN2_46:
	case MZ2_TANK_MACHINEGUN2_47:
	case MZ2_TANK_MACHINEGUN2_48:
	case MZ2_TANK_MACHINEGUN2_49:
	case MZ2_TANK_MACHINEGUN2_50:
	case MZ2_TANK_MACHINEGUN2_51:
	case MZ2_TANK_MACHINEGUN2_52:
	case MZ2_TANK_MACHINEGUN2_53:
	case MZ2_TANK_MACHINEGUN2_54:
	
		gi.sound(self, CHAN_WEAPON, gi.soundindex(va("tank/tnkatk2%c.wav", 'a' + rand() % 5)), 1, ATTN_NORM, 0);
		//Com_sprintf(soundname, sizeof(soundname), "tank/tnkatk2%c.wav", 'a' + rand() % 5);
		break;

	case MZ2_CHICK_ROCKET_1:
	case MZ2_TURRET_ROCKET:			// PGM
		gi.sound(self, CHAN_WEAPON, gi.soundindex("chick/chkatck2.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_TANK_ROCKET_1:
	case MZ2_TANK_ROCKET_2:
	case MZ2_TANK_ROCKET_3:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("tank/tnkatck1.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_SUPERTANK_ROCKET_1:
	case MZ2_SUPERTANK_ROCKET_2:
	case MZ2_SUPERTANK_ROCKET_3:
	case MZ2_BOSS2_ROCKET_1:
	case MZ2_BOSS2_ROCKET_2:
	case MZ2_BOSS2_ROCKET_3:
	case MZ2_BOSS2_ROCKET_4:
	case MZ2_CARRIER_ROCKET_1:
		//	case MZ2_CARRIER_ROCKET_2:
		//	case MZ2_CARRIER_ROCKET_3:
		//	case MZ2_CARRIER_ROCKET_4:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("tank/rocket.wav"), 1.0, ATTN_NORM, 0);

		break;

	case MZ2_GUNNER_GRENADE_1:
	case MZ2_GUNNER_GRENADE_2:
	case MZ2_GUNNER_GRENADE_3:
	case MZ2_GUNNER_GRENADE_4:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("gunner/gunatck3.wav"), 1.0, ATTN_NORM, 0);
		break;

	case MZ2_JORG_MACHINEGUN_L1:
	case MZ2_JORG_MACHINEGUN_L2:
	case MZ2_JORG_MACHINEGUN_L3:
	case MZ2_JORG_MACHINEGUN_L4:
	case MZ2_JORG_MACHINEGUN_L5:
	case MZ2_JORG_MACHINEGUN_L6:
		gi.sound(self, CHAN_WEAPON, gi.soundindex("boss3/xfire.wav"), 1.0, ATTN_NORM, 0);
		break;


	}

}
void muzzleflash_think(edict_t *self)
{

	if (self->s.frame == 1)
		G_FreeEdict(self);
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
}

void spawn_m_muzzleflash(edict_t *self, vec3_t start, vec3_t dir, int flashtype)
{
	edict_t *flash;
	flash = G_Spawn();
	flash->think = muzzleflash_think;
	flash->nextthink = level.time + FRAMETIME;
	VectorCopy(start, flash->s.origin);
	vectoangles(dir, flash->s.angles);
	//VectorMA(flash->s.origin, FRAMETIME, self->velocity, flash->s.origin);
	flash->movetype = MOVETYPE_FLY;
	//VectorCopy(self->velocity, flash->velocity);
	//VectorMA(flash->velocity, 0.75, self->velocity, flash->velocity);
	flash->s.angles[2] = crandom() * 999;
	flash->s.renderfx = (RF_TRANSLUCENT | RF_FULLBRIGHT | RF_NOSHADOW | RF_MINLIGHT | RF_VIEWERMODEL);
	flash->owner = self;
	gi.setmodel(flash, "models/objects/mflash/tris.md2");

	gi.linkentity(flash);
	
	/*gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_WELDING_SPARKS);
	gi.WriteByte(25);
	gi.WritePosition(tr.endpos);
	gi.WriteDir(tr.plane.normal);
	gi.WriteByte(0xe0);*/
	if (flashtype)
	{
		shot_sound(self, flashtype);
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(self - g_edicts);
		gi.WriteByte(MZ_NUKE2);
		gi.multicast(start, MULTICAST_PVS);
	}
}

float get_dist(edict_t *self, edict_t *other)
{
	vec3_t dist;
	VectorSubtract(self->s.origin, other->s.origin, dist);
	return VectorLength(dist);
}

void monster_skip_frame(edict_t *self)
{
	if (skill->value <= 3)
		return;
	self->monsterinfo.nextframe = self->s.frame + 2;
}

void add_sp_score(edict_t *self, int amount, double type)
{
	if (!self->client || deathmatch->value)
		return;

	self->client->resp.score += amount * type;
	if (type == SCORE_DAMAGE_RECEIVED)
	{
		self->client->resp.score_dmg_received += amount * type;
	}
	else if (type == SCORE_DAMAGE_DEALT)
	{
		self->client->resp.score_dmg_dealt += amount * type;
	}
	else if (type == SCORE_DAMAGE_SAVED)
	{
		self->client->resp.score_dmg_saved += amount * type;
	}
	else if (type == SCORE_KILLS)
	{
		self->client->resp.score_kills += amount * type;
	}
	else if (type == SCORE_ITEM_PICKUP)
	{
		self->client->resp.score_item_pickup += amount * type;
	}
	else if (type == SCORE_ITEM_USAGE)
	{
		self->client->resp.score_item_usage += amount * type;
	}
	else if (type == SCORE_ITEM_HEALTH_BONUS)
	{
		self->client->resp.score_health_bonus += amount * type;
	}
	else if (type == SCORE_OBJECTIVES)
	{
		self->client->resp.score_objectives += amount * type;
	}
	else if (type == SCORE_SECRETS)
	{
		self->client->resp.score_secrets += amount * type;
	}

}

float scan_dir(edict_t *self, int dir, float dist, vec3_t result)
{
	trace_t tr;
	vec3_t dirv;
	vec3_t distance;
	vec3_t forward, right, up, end;
	vec3_t angles;
	//VectorCopy(self->s.angles, angles);



	AngleVectors(self->s.angles, forward, right, up);
	if (dir == SCAN_LEFT)
		VectorScale(right, -1, end);
	else if (dir == SCAN_RIGHT)
		VectorCopy(right, end);
	else if (dir == SCAN_FORWARD)
		VectorCopy(forward, end);
	else if (dir == SCAN_BACKWARDS)
		VectorScale(forward, -1, end);
	else if (dir == SCAN_UP)
		VectorCopy(up, end);
	else if (dir == SCAN_DOWN)
		VectorScale(up, -1, end);

	VectorMA(self->s.origin, dist, end, end);
	//gi.bprintf(PRINT_HIGH, "scan_dir: angles = %s, forward = %f %f %f, right = %f %f %f, up = %f %f %f, end = %f %f %f\n", vtos(angles), forward[0], forward[1], forward[2], right[0], right[1], right[2], up[0], up[1], up[2], end[0], end[1], end[2]);
	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);

	VectorCopy(tr.endpos, result);
	//VectorCopy(tr.endpos, self->s.origin);
	//gi.linkentity(self);
	return tr.fraction;
}

