#include "g_local.h"

#ifdef WESQ2

void SP_misc_tank(edict_t *self)
{
	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");
	gi.modelindex ("models/objects/debris3/tris.md2");

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	if(!self->mass)
		self->mass = 50;
	if(!self->health)
		self->health = 10;
	if(!self->dmg)
		self->dmg = 20;
	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->think = M_droptofloor;
	self->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (self);
}

void SP_misc_tank1 (edict_t *self)
{
	self->model = "models/objects/tank1/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 62);
	SP_misc_tank(self);
}
void SP_misc_tank2 (edict_t *self)
{
	self->model = "models/objects/tank2/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -48, -48, 0);
	VectorSet (self->maxs, 48, 48, 185);
	SP_misc_tank(self);
}
void SP_misc_ladder (edict_t *self)
{
	self->model = "models/objects/ladder/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins,-16,-16,  0);
	VectorSet (self->maxs, 16, 16,192);
	self->solid    = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	self->think    = M_droptofloor;
	self->nextthink= level.time + 2 * FRAMETIME;
	self->clipmask = MASK_PLAYERSOLID | CONTENTS_LADDER;
	gi.linkentity(self);

}

void bomb_think (edict_t *self)
{
	vec3_t	spark_origin;
	vec3_t	movedir;

	VectorCopy(self->s.origin,spark_origin);
	spark_origin[2] += 16;
	VectorSet(movedir,0,0,1);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (spark_origin);
	gi.WriteDir (movedir);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_bomb(edict_t *self)
{
	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");
	gi.modelindex ("models/objects/debris3/tris.md2");

	self->model = "models/objects/boom/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -12, -12, -12);
	VectorSet (self->maxs,  12,  12,  14);
	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;
	if(!self->health)
		self->health = 10;
	if(!self->dmg)
		self->dmg = 20;
	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->think = bomb_think;
	self->nextthink = level.time + 2*FRAMETIME;
	gi.linkentity (self);
}

#endif
