#include "g_local.h"

void cutbox_attack (edict_t *self)
{
	edict_t *ent;
	ent = NULL;

	//while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
	while ((ent = findradius(ent, self->s.origin, 300)) != NULL)
	{
		if (ent->client)
		{
			vec3_t	start, dir, end;
			vec3_t	forward, right;

			AngleVectors (self->s.angles, forward, right, NULL);
			G_ProjectSource (self->s.origin, monster_flash_offset[0], forward, right, start);

			VectorCopy (ent->s.origin, end);
			end[2] += ent->viewheight;
			//VectorSubtract (end, start, dir);

			VectorSubtract (end, start, dir);
			VectorNormalize (dir);


			//fire cutterbox
			fire_cutterbox (self, self->s.origin, dir, 15, 175);
		}
	}

	self->nextthink = level.time + 0.55;
}

void SP_monster_cutbox (edict_t *self)
{
	//will not remove in deathmatch

	//no model, doesn't move, not solid, can't be damaged
	//self->s.modelindex = NULL;
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->takedamage = false;

	//attack think every 0.5 seconds
	self->think = cutbox_attack;
	self->nextthink = level.time + 0.55;


	//put in entity que
	gi.linkentity(self);

}
