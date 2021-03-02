#include "../g_local.h"

void CloneStuff (edict_t *self)
{
	vec3_t clone1_origin;
	vec3_t clone1_add = {40, 0, 0};

	VectorCopy (self->owner->s.origin, clone1_origin);
	_VectorAdd (clone1_origin, clone1_add, clone1_origin);

	VectorCopy (clone1_origin, self->s.origin);

	self->s.angles[0] = self->owner->s.angles[0];
	self->s.angles[1] = self->owner->s.angles[1];
	self->s.angles[2] = self->owner->s.angles[2];

	self->s.frame = self->owner->s.frame;

	self->nextthink = level.time + .1;

	self->model = self->owner->model;
	self->s.skinnum = self->owner->s.skinnum;
	self->s.modelindex = self->owner->s.modelindex;
	self->s.modelindex2 = self->owner->s.modelindex;
	self->s.effects = self->owner->s.effects;
	self->s.renderfx = self->owner->s.renderfx;

	gi.linkentity (self);
}

void CloneStuff2 (edict_t *self)
{
	vec3_t clone1_origin;
	vec3_t clone1_add = {0, 40, 0};

	VectorCopy (self->owner->s.origin, clone1_origin);
	_VectorAdd (clone1_origin, clone1_add, clone1_origin);

	VectorCopy (clone1_origin, self->s.origin);

	self->s.angles[0] = self->owner->s.angles[0];
	self->s.angles[1] = self->owner->s.angles[1];
	self->s.angles[2] = self->owner->s.angles[2];

	self->s.frame = self->owner->s.frame;

	self->nextthink = level.time + .1;

	self->model = self->owner->model;
	self->s.skinnum = self->owner->s.skinnum;
	self->s.modelindex = self->owner->s.modelindex;
	self->s.modelindex2 = self->owner->s.modelindex;
	self->s.effects = self->owner->s.effects;
	self->s.renderfx = self->owner->s.renderfx;

	gi.linkentity (self);
}

void CloneStuff3 (edict_t *self)
{
	vec3_t clone1_origin;
	vec3_t clone1_add = {-40, 0, 0};

	VectorCopy (self->owner->s.origin, clone1_origin);
	_VectorAdd (clone1_origin, clone1_add, clone1_origin);

	VectorCopy (clone1_origin, self->s.origin);

	self->s.angles[0] = self->owner->s.angles[0];
	self->s.angles[1] = self->owner->s.angles[1];
	self->s.angles[2] = self->owner->s.angles[2];

	self->s.frame = self->owner->s.frame;

	self->nextthink = level.time + .1;

	self->model = self->owner->model;
	self->s.skinnum = self->owner->s.skinnum;
	self->s.modelindex = self->owner->s.modelindex;
	self->s.modelindex2 = self->owner->s.modelindex;
	self->s.effects = self->owner->s.effects;
	self->s.renderfx = self->owner->s.renderfx;

	gi.linkentity (self);
}

void CloneStuff4 (edict_t *self)
{
	vec3_t clone1_origin;
	vec3_t clone1_add = {0, -40, 0};

	VectorCopy (self->owner->s.origin, clone1_origin);
	_VectorAdd (clone1_origin, clone1_add, clone1_origin);

	VectorCopy (clone1_origin, self->s.origin);

	self->s.angles[0] = self->owner->s.angles[0];
	self->s.angles[1] = self->owner->s.angles[1];
	self->s.angles[2] = self->owner->s.angles[2];

	self->s.frame = self->owner->s.frame;

	self->nextthink = level.time + .1;

	self->model = self->owner->model;
	self->s.skinnum = self->owner->s.skinnum;
	self->s.modelindex = self->owner->s.modelindex;
	self->s.modelindex2 = self->owner->s.modelindex;
	self->s.effects = self->owner->s.effects;
	self->s.renderfx = self->owner->s.renderfx;

	gi.linkentity (self);
}

void CreateClones (edict_t *ent)
{
	edict_t *clone1;
	vec3_t clone1_origin;
	vec3_t clone1_add = {40, 0, 0};
	edict_t *clone2;
	vec3_t clone2_origin;
	vec3_t clone2_add = {0, 40, 0};
	edict_t *clone3;
	vec3_t clone3_origin;
	vec3_t clone3_add = {-40, 0, 0};
	edict_t *clone4;
	vec3_t clone4_origin;
	vec3_t clone4_add = {0, -40, 0};

	if (CheckBan(feature_ban, FEATUREBAN_CLONESHIELD))
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, Clone Shield is banned.\n");
		return;
	}

	VectorCopy (ent->s.origin, clone1_origin);
	_VectorAdd (clone1_origin, clone1_add, clone1_origin);

	clone1 = CreateEntity (clone1_origin, ent->s.angles, MOVETYPE_NONE, 0, SOLID_NOT, 0, 0, 0, NULL, NULL, ent);
	clone1->model = ent->model;
	clone1->s.skinnum = ent->s.skinnum;
	clone1->s.modelindex = ent->s.modelindex;
	clone1->s.modelindex2 = ent->s.modelindex;
	clone1->think = CloneStuff;
	clone1->nextthink = level.time + .1;

	gi.linkentity (clone1);

	VectorCopy (ent->s.origin, clone2_origin);
	_VectorAdd (clone2_origin, clone2_add, clone2_origin);

	clone2 = CreateEntity (clone2_origin, ent->s.angles, MOVETYPE_NONE, 0, SOLID_NOT, 0, 0, 0, NULL, NULL, ent);
	clone2->model = ent->model;
	clone2->s.skinnum = ent->s.skinnum;
	clone2->s.modelindex = ent->s.modelindex;
	clone2->s.modelindex2 = ent->s.modelindex;
	clone2->think = CloneStuff2;
	clone2->nextthink = level.time + .1;

	gi.linkentity (clone2);

	VectorCopy (ent->s.origin, clone3_origin);
	_VectorAdd (clone3_origin, clone3_add, clone3_origin);

	clone3 = CreateEntity (clone3_origin, ent->s.angles, MOVETYPE_NONE, 0, SOLID_NOT, 0, 0, 0, NULL, NULL, ent);
	clone3->model = ent->model;
	clone3->s.skinnum = ent->s.skinnum;
	clone3->s.modelindex = ent->s.modelindex;
	clone3->s.modelindex2 = ent->s.modelindex;
	clone3->think = CloneStuff3;
	clone3->nextthink = level.time + .1;

	gi.linkentity (clone3);

	VectorCopy (ent->s.origin, clone4_origin);
	_VectorAdd (clone4_origin, clone4_add, clone4_origin);

	clone4 = CreateEntity (clone4_origin, ent->s.angles, MOVETYPE_NONE, 0, SOLID_NOT, 0, 0, 0, NULL, NULL, ent);
	clone4->model = ent->model;
	clone4->s.skinnum = ent->s.skinnum;
	clone4->s.modelindex = ent->s.modelindex;
	clone4->s.modelindex2 = ent->s.modelindex;
	clone4->think = CloneStuff4;
	clone4->nextthink = level.time + .1;

	gi.linkentity (clone4);

	ent->client->clone1 = clone1;
	ent->client->clone2 = clone2;
	ent->client->clone3 = clone3;
	ent->client->clone4 = clone4;
}
