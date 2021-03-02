// s_weapons.c

#include "g_local.h"
#include "laser.h"
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

/*
=================
bolt_touch

=================
*/
void bolt_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	origin;

	// can't be hit by own rocket
	if (other == ent->owner)
		return;

	// can't hit sky
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// noise if hits other player
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (random() < 0.5)
	{
		if (!plane)
			G_ImpactEntity (TE_ELECTRIC_SPARKS, ent->s.origin, ent->s.origin, MULTICAST_PVS);
		else
			G_ImpactEntity (TE_ELECTRIC_SPARKS, ent->s.origin, plane->normal, MULTICAST_PVS);
	}
	else
	{
		if (!plane)
			G_SplashEntity (TE_LASER_SPARKS, 2400, ent->s.origin, vec3_origin, 0x0000000A, MULTICAST_PVS);
		else
			G_SplashEntity (TE_LASER_SPARKS, 2400, ent->s.origin, plane->normal, 0x0000000A, MULTICAST_PVS);
	}

	gi.sound(ent, CHAN_VOICE, gi.soundindex("world/spark3.wav"), 1, ATTN_NONE,0);

	G_FreeEdict (ent);
}

void fire_el (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*bolt;

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (dir, bolt->movedir);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	(rand() & 1) ? (bolt->s.effects = EF_GREENGIB) : (bolt->s.effects |= (EF_GRENADE | EF_COLOR_SHELL | RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE));
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
    bolt->s.modelindex3 = 0;
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->dmg = damage;
	bolt->classname = "fbolt";

	gi.linkentity (bolt);
}

void fire_bolt (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*bolt;

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (dir, bolt->movedir);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;

	if (rand() & 1)
		bolt->s.effects = EF_GREENGIB;
	else
		bolt->s.effects = EF_GRENADE;

	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
    bolt->s.modelindex3 = 0;
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->dmg = damage;
	bolt->classname = "fbolt";

	gi.linkentity (bolt);
}

/*
=================
fire_electric
=================
*/
void fire_electric (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	tr = gi.trace (from, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent != self) && (tr.ent->takedamage))
		T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_ELECTRIC);

	VectorCopy (tr.endpos, from);

	if (G_ClientExists(self))
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return;

    G_ImpactEntity (TE_FLECHETTE, tr.endpos, tr.plane.normal, MULTICAST_PVS);
}
