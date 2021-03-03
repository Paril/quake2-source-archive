


#if 0 /* Turret code disabled */



#include "g_local.h"

void annturret_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void annturret_think(edict_t *self);
void annturret_blocked(edict_t *self, edict_t *other);
void annturret_init(edict_t *self);

void SP_annturret_body(edict_t *self)
{
	self->solid = SOLID_BSP;
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);

	if (!self->speed)
		self->speed = 3.0;
	if (!self->accel)
		self->accel = 0.5;
	if (!self->dmg)
		self->dmg = 50.0;

	self->think = annturret_init;
	self->nextthink = level.time + 0.5;
	self->blocked = annturret_blocked;
	self->touch = annturret_touch;

	gi.linkentity (self);
}

void annturret_init(edict_t *self)
{
/* *** EVIL HACK! REMOVE THIS BEFORE 1.22! *** */
if (Q_stricmp(level.mapname,"annctf2")==0)
{
G_FreeEdict(self);
return;
}
/* *** */
	if (!self->target)
	{
		gi.dprintf("%s at %s needs a target\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);

		return;
	}

	self->target_ent = G_PickTarget (self->target);
	VectorSubtract (self->target_ent->s.origin, self->s.origin, self->move_origin);
	G_FreeEdict(self->target_ent);

	self->think = annturret_think;
	self->nextthink = level.time + FRAMETIME;
}

void annturret_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->client)
	{
		ent->parent=other;
		other->parent=ent;

		ent->tomvar=0;
	}
}

void annturret_think(edict_t *self)
{
	if (self->tomvar<5)
		self->tomvar++;
	else
		self->parent=NULL;

	if (!self->parent)
	{
		if (self->pos1[YAW]>0)
			self->pos1[YAW]-=1;
		else if (self->pos1[YAW]<0)
			self->pos1[YAW]+=1;

		self->pos1[YAW]=(float)((int)self->pos1[YAW]);
	}

	VectorCopy(self->s.angles,self->pos2);

	if (self->pos1[YAW]!=0)
	{
		self->s.angles[YAW]+=self->pos1[YAW];
		if (self->s.angles[YAW]<0)
			self->s.angles[YAW]=360+self->s.angles[YAW];
		else if (self->s.angles[YAW]>360)
			self->s.angles[YAW]=self->s.angles[YAW]-360;
	}

	if (self->s.angles[PITCH]<0)
		self->s.angles[PITCH]+=2;

	self->nextthink = level.time + FRAMETIME;
}

void annturret_blocked(edict_t *self, edict_t *other)
{
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, 0, 1, 0, 0);
	VectorCopy(self->pos2,self->s.angles);
}

void annturret_left(edict_t *ent)
{
	if (ent->parent && ent->parent->parent==ent)
	{
		ent->parent->pos1[YAW]+=ent->parent->accel;
		if (ent->parent->pos1[YAW]>ent->parent->speed)
			ent->parent->pos1[YAW]=ent->parent->speed;
	}
	else
	{
		ent->parent=NULL;
	}
}

void annturret_right(edict_t *ent)
{
	if (ent->parent && ent->parent->parent==ent)
	{
		ent->parent->pos1[YAW]-=ent->parent->accel;
		if (ent->parent->pos1[YAW]<-ent->parent->speed)
			ent->parent->pos1[YAW]=-ent->parent->speed;
	}
	else
	{
		ent->parent=NULL;
	}
}

void annturret_fire(edict_t *ent)
{
	vec3_t f,r,u,start;

	if (ent->parent && ent->parent->parent==ent && ent->parent->s.angles[PITCH]==0)
	{
    	AngleVectors (ent->parent->s.angles, f, r, u);
		VectorMA (ent->parent->s.origin, ent->parent->move_origin[0], f, start);
		VectorMA (start, ent->parent->move_origin[1], r, start);
		VectorMA (start, ent->parent->move_origin[2], u, start);

		fire_rocket (ent, start, f, ent->parent->dmg, (750 + 50 * skill->value), 200, (750 + 50 * skill->value));
		gi.positioned_sound (start, ent->parent, CHAN_WEAPON, gi.soundindex("weapons/rocklf1a.wav"), 1, ATTN_NORM, 0);

		ent->parent->s.angles[PITCH]=-10;
	}
	else
	{
		ent->parent=NULL;
	}
}



#endif
