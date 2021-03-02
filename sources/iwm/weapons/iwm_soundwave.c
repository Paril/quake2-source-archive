#include "../g_local.h"

void sndwve_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

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
		mod = MOD_SHOCKWAVE; // MOD FIXME
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}

	//G_FreeEdict (self);
}

void SoundWave_WaveTheSound (edict_t *e)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, e->s.origin, 400))
	{
		if (!blip->takedamage)
			continue;
		//if (blip == e->owner)
		//	continue;

		if (!blip->client)
		{
			// Non-client entities. No need for MeansOfDeath here.
			T_Damage (blip, e, e->owner, vec3_origin, blip->s.origin, vec3_origin, 10, 0, 0, MOD_SHOCKWAVE);
		}
		else
		{
			if (blip->headache_framenum < level.framenum)
			{
				blip->client->blindTime = 5;
				blip->client->blindBase = 5;
				blip->headache_framenum = level.framenum + 50;
			}

			T_Damage (blip, e, e->owner, vec3_origin, blip->s.origin, vec3_origin, 10, 0, 0, MOD_SHOCKWAVE); // MOD FIXME
		}
	}
	e->nextthink = level.time + .1;
}

void SoundWave_Close (edict_t *e)
{
	e->s.frame++;

	if (e->s.frame == 15)
	{
		G_FreeEdict (e);
		return;
	}

	e->nextthink2 = level.time + .1;
}

void SoundWave_Open (edict_t *e)
{
	e->s.frame++;

	if (e->s.frame == 11)
	{
		e->nextthink2 = level.time + 1;
		e->think2 = SoundWave_Close;
		return;
	}

	e->nextthink2 = level.time + .1;
}

void fire_soundwave (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
//	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	//bolt->s.angles[1] += 90;
	bolt->movetype = MOVETYPE_WALLBOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_SPHERETRANS;
	bolt->s.renderfx |= RF_TRANSLUCENT;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/proj/soundwave/tris.md2");
	bolt->owner = self;
	bolt->touch = sndwve_touch;
	bolt->nextthink = level.time + 1;
	bolt->think = SoundWave_WaveTheSound;
	bolt->dmg = damage;
	bolt->classname = "sndwve";
	bolt->nextthink2 = level.time + .1;
	bolt->think2 = SoundWave_Open;
	bolt->s.sound = SoundIndex ("weapons/annoy.wav");
	gi.linkentity (bolt);
	bolt->can_teleport_through_destinations = 1;
	bolt->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	/*tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}*/
}	


void Weapon_Soundwave_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

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
	//fire_bfg (ent, start, forward, damage, 400, damage_radius);
	fire_soundwave (ent, start, forward, 5, 600);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_Soundwave (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, Weapon_Soundwave_Fire);
}
