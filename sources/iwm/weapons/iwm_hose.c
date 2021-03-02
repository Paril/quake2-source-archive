#include "../g_local.h"

void water2_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}
	
	if (other->takedamage)
	{
		vec3_t	kvel;
		float	mass;
		
		if (other->mass < 50)
			mass = 50;
		else
			mass = other->mass;
		
		VectorScale (self->velocity, 180 / mass, kvel);
		
		VectorAdd (other->velocity, kvel, other->velocity);
		G_FreeEdict(self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	G_FreeEdict(self);
}

void SpawnSparkies (edict_t *self);

void fire_water (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_BOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("sprites/null.sp2");
	bolt->owner = self;
	bolt->touch = water2_touch;
	bolt->think = SpawnSparkies;
	bolt->nextthink = level.time + 0.1;
	bolt->think2 = G_FreeEdict;
	bolt->nextthink2 = level.time + 4;
	bolt->classname = "water";
	gi.linkentity (bolt);
	bolt->can_teleport_through_destinations = 1;
	bolt->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


void Weapon_Hose_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	//fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
	//ThrowWater(ent, "sprites/null.sp2", 1600, start, forward);
	fire_water (ent, start, forward, 1800);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_SHOTGUN | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Hose (edict_t *ent)
{
	static int	pause_frames[]	= {7, 9, 0};
	static int	fire_frames[]	= {5, 6, 0};

	Weapon_Generic (ent, 3, 6, 9, 13, pause_frames, fire_frames, Weapon_Hose_Fire);

	if (ent->client->ps.gunframe == 6 && ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
		ent->client->ps.gunframe = 5;
}
