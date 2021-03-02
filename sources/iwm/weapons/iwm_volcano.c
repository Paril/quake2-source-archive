#include "../g_local.h"

void lav_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	vec3_t origin;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		mod = MOD_LAVABALL;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
	VectorMA (self->s.origin, -0.02, self->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
	{
		if (self->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (self->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//		G_Spawn_Sparks (TE_TUNNEL_SPARKS, self->s.origin, vec3_origin, self->s.origin);
		//G_Spawn_Splash(TE_TUNNEL_SPARKS, 6, COLOR_RED, self->s.origin, vec3_origin, self->s.origin);
	}

	T_RadiusDamage(self, self->owner, 200, other, 400, MOD_LAVABALL_SPLASH);

	G_FreeEdict (self);
}

void fire_lav (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effec)
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
	bolt->movetype = MOVETYPE_TOSS;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_COLOR_SHELL | EF_DOUBLE | EF_GRENADE;
	bolt->s.renderfx |= RF_SHELL_RED;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/proj/lavaball/tris.md2");
	bolt->owner = self;
	bolt->touch = lav_touch;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	bolt->can_teleport_through_destinations = 1;
	bolt->s.renderfx |= RF_IR_VISIBLE;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

void Lava_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void LavaKerplode (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	Lava_Explode (ent);
}

void SpewFire (edict_t *e)
{
	vec3_t dir = {(rand()%20 - rand()%20), (rand()%20 - rand()%20), rand()%360};
	trace_t tr;
	vec3_t start, end, up, out;
	float dist;

	VectorCopy (e->s.origin, start);
	AngleVectors (e->s.angles, NULL, NULL, up);
	VectorMA (start, 8192, up, end);
	tr = gi.trace (start, NULL, NULL, end, e, MASK_SOLID);
	VectorSubtract (start, tr.endpos, out);
	dist = VectorLength(out);
	if (dist < 1755)
		dist = 1755;


	fire_lav (e->owner, e->s.origin, dir, 15, (int)dist-(dist/3), 0);

	e->nextthink = level.time + .2;
}

void Grow (edict_t *e)
{
	e->s.frame++;

	if (e->s.frame == 5)
	{
		e->think = SpewFire;
		e->nextthink = level.time + 0.4;
	}
	e->nextthink = level.time + .1;
}

void TestSee (edict_t *e)
{
	//gi.dprintf ("%f\n", e->avelocity[1]);
	e->nextthink2 = level.time + .1;
}

void SpawnVolcano (edict_t *ent)
{
	edict_t *vol;
	vec3_t angles = {0, 0, 0};
	vec3_t mins = {48, 48, 48};
	vec3_t maxs = {-48, -48, -48};

	vol = CreateEntity (ent->s.origin, angles, MOVETYPE_NONE, 0, SOLID_BBOX, 0, 0, 1, mins, maxs, ent->owner);

	vol->s.modelindex = ModelIndex("models/other/volcano/tris.md2");
	vol->s.frame = 0;
	vol->think = Grow;
	vol->nextthink = level.time + .1;
	vol->owner = ent->owner;
	vol->solid = SOLID_BBOX;
	vol->think2 = G_FreeEdict;
	vol->nextthink2 = level.time + 10;
	VectorSet (vol->mins, 48, 48, 48);
	VectorSet (vol->maxs, -48, -48, -48);

	gi.linkentity (vol);
}

void VolcanoGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	if (ent->avelocity[1])
	{
		ent->nextthink = level.time + .1;
		return;
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	SpawnVolcano (ent);

	G_FreeEdict (ent);
}

void VolcanoGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	VolcanoGrenade_Explode (ent);
}

void fire_volcanogrenade (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	self->s.angles[2] = 0;
	self->s.angles[0] = 0;
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	VectorSet (bfg->avelocity, 300, 300, 300);
	bfg->movetype = MOVETYPE_BOUNCE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = ModelIndex ("models/objects/grenade/tris.md2");
	bfg->owner = self;
	bfg->touch = VolcanoGrenade_Touch;
	bfg->nextthink = level.time + 2.5;
	bfg->think = VolcanoGrenade_Explode;
	bfg->classname = "volcanogrenade";
	bfg->can_teleport_through_destinations = 1;
	bfg->s.renderfx |= RF_IR_VISIBLE;


	gi.linkentity (bfg);
}


void Weapon_Volcano_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->volcanoed  && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		safe_cprintf (ent, PRINT_HIGH, "You already volcanoed this life!\n");
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	fire_volcanogrenade (ent, start, forward, 25, 800, 0);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;

	ent->client->volcanoed = 1;
}

void Weapon_Volcano (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, Weapon_Volcano_Fire);
}
