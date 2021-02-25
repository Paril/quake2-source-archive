#include "g_local.h"


/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if (!tr.ent->enemy)
		return;
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		if (tr.ent->team_owner == self->team_owner)
			return;
		if (tr.ent->activator == self->goalentity)
			return;
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta, NULL);
	}
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	//see if enemy is in range
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->mins[0] && aim[1] < self->maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->enemy->mins[0];
		else
			aim[1] = self->enemy->maxs[0];
	}

	VectorMA (self->s.origin, range, dir, point);

	tr = gi.trace (self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;
	return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else if (self->spawnflags & 2)
			mod = MOD_MEDICHYPER;
		else if (self->spawnflags & 4)
			mod = MOD_TANKBLAST;
		else if (self->spawnflags & 8)
			mod = MOD_SOLDBLAST;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		if (self->owner->client)
		{
			if (self->owner->client->pers.abilities.icarus_explobolts)
			{
				gi.WriteByte (svc_temp_entity);
				if (self->waterlevel)
					gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
				else
					gi.WriteByte (TE_ROCKET_EXPLOSION);
				gi.WritePosition (self->s.origin);
				gi.multicast (self->s.origin, MULTICAST_PHS);
			}
			else
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BLASTER);
				gi.WritePosition (self->s.origin);
				if (!plane)
					gi.WriteDir (vec3_origin);
				else
					gi.WriteDir (plane->normal);
				gi.multicast (self->s.origin, MULTICAST_PVS);
			}
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BLASTER);
			gi.WritePosition (self->s.origin);
			if (!plane)
				gi.WriteDir (vec3_origin);
			else
				gi.WriteDir (plane->normal);
			gi.multicast (self->s.origin, MULTICAST_PVS);
		}
	}

	if (self->owner->client)
	{
		if (self->owner->client->pers.abilities.icarus_explobolts)
			T_RadiusDamage (self, self->owner, 50, NULL, 100, MOD_BLASTER);
	}

	G_FreeEdict (self);
}

// Glub.
// Paril FIXME: Use someday.
void Object_Drunkify (edict_t *drinker)
{
	int p = (rand() % 65 - rand() % 65);
	int o = (rand() % 65 - rand() % 65);
	int i = (rand() % 65 - rand() % 65);

	drinker->s.angles[0] += p;
	drinker->s.angles[1] += o;
	drinker->s.angles[2] += i;

	drinker->velocity[0] += p;
	drinker->velocity[1] += o;
	drinker->velocity[2] += i;

	drinker->nextthink = level.time + 0.1;
}


void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int which_mod)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE; // special net code is used for projectiles
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	if (self->client)
	{
		if (self->client->pers.abilities.icarus_explobolts)
		{
			bolt->s.effects |= effect | EF_PLASMA;
		}
		else
			bolt->s.effects |= effect;
	}
	else
		bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;

	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (which_mod = 1)
		bolt->spawnflags = 1;
	else if (which_mod = 2)
		bolt->spawnflags = 2;
	else if (which_mod = 3)
		bolt->spawnflags = 4;
	else if (which_mod = 4)
		bolt->spawnflags = 8;
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


/*
=================
fire_grenade
=================
*/
static void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

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

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

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
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}


/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (ent->owner->client)
	{
		if (ent->owner->client->pers.abilities.dchick_plasma)
		{
			ent->dmg = ent->dmg * 12;
		}
	}

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				//while(n--)
//					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}


	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	if (ent->owner->client)
	{
		if (ent->owner->client->pers.abilities.dchick_plasma)
		{
			gi.positioned_sound(ent->s.origin,ent,CHAN_WEAPON,gi.soundindex("makron/bfg_fire.wav"),1,ATTN_NORM,0);

			VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			if (ent->waterlevel)
				gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
			else
				gi.WriteByte (TE_ROCKET_EXPLOSION);
			gi.WritePosition (origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
		}
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		if (ent->waterlevel)
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (origin);
		gi.multicast (ent->s.origin, MULTICAST_PHS);
	}

	G_FreeEdict (ent);
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*
=================
fire_rail
=================
*/
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;
	int             n;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
				(tr.ent->solid == SOLID_BBOX))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_ENERGY, mod);
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

	if (self->client)
	{
	if (self->client->pers.abilities.gladiator_wallpierce)
	{
        // Take this point, and add 1, until we find an open place...
        VectorMA (tr.endpos, 10, aimdir, from);
        n = 0;
        while (n < 10)
        {
        if ((!(gi.pointcontents(from) & CONTENTS_SOLID)) )
        {
                    // Found a hole...fire again...
                    fire_rail (self, from, aimdir, damage, kick, MOD_RAILGUN);
                    n = 10;
        }
        else
                    n++;
                    VectorMA (from, 10, aimdir, from);
        }

	}
	}
}


/*
=================
fire_bfg
=================
*/
void bfg_explode (edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->owner))
				continue;

			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
	T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}


void bfg_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

//ZOID
		//don't target players in CTF
		if (ctf->value && ent->client &&
			self->owner->client &&
			ent->client->resp.ctf_team == self->owner->client->resp.ctf_team)
			continue;
//ZOID

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
}

void Bfg_Think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
	if (++self->s.frame < 9)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	//bfg->s.effects |= EF_BFG;// | EF_ANIM_ALLFAST;
	bfg->s.frame = 0;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("models/objects/bfgball/tris.md2"/*"sprites/s_bfg1.sp2"*/);
	bfg->owner = self;
	bfg->touch = bfg_touch;

	bfg->nextthink = level.time + 6000;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	bfg->think = Bfg_Think;
	bfg->nextthink = level.time + 2 * FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}

void vectoangles2 (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
	// PMM - fixed to correct for pitch of 0
		if (value1[0])
			yaw = (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = 270;

		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

void fire_player_melee (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("infantry/melee2.wav"), 1, ATTN_NORM, 0);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);
	}
}

void fire_player_melee_stalk (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("stalker/melee1.wav"), 1, ATTN_NORM, 0);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);
	}
}

void fire_player_melee_glad (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);
		gi.sound (self, CHAN_AUTO, gi.soundindex ("gladiator/melee2.wav"), 1, ATTN_NORM, 0);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		/*VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);*/
	}
}

void fire_player_melee_brain (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);
		gi.sound (self, CHAN_AUTO, gi.soundindex ("brain/melee3.wav"), 1, ATTN_NORM, 0);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		/*VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);*/
	}
}

void fire_player_melee_berserk (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick, DAMAGE_NO_ARMOR, mod);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		/*VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);*/
	}
}


void fire_player_melee_mutant (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	if (self->client->resp.mutant_jumped == 1)
		return;
	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		if (tr.ent->takedamage)
			self->client->resp.mutant_jumped = 1;
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick, DAMAGE_NO_ARMOR, mod);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		/*VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);*/
	}
}

void fire_player_melee_mutant2 (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	if (self->client->resp.mutant_jumped == 1)
		return;
	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		if (tr.ent->takedamage)
			self->client->resp.mutant_jumped = 1;
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick, DAMAGE_NO_ARMOR, mod);
	gi.sound (self, CHAN_WEAPON, gi.soundindex ("mutant/Mutatck2.wav"), 1, ATTN_NORM, 0);
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		/*VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);*/
	}
}

void fire_player_melee_flyer (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		/*VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);*/
	}
}

// Mission Pack 1

/*
==========
Gekk Spit
==========
*/
void loogie_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_BLASTER);
	
	G_FreeEdict (self);
};
	


void fire_loogie (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*loogie;
	trace_t	tr;

	VectorNormalize (dir);

	loogie = G_Spawn();
	VectorCopy (start, loogie->s.origin);
	VectorCopy (start, loogie->s.old_origin);
	vectoangles (dir, loogie->s.angles);
	VectorScale (dir, speed, loogie->velocity);
	loogie->movetype = MOVETYPE_FLYMISSILE;
	loogie->clipmask = MASK_SHOT;
	loogie->solid = SOLID_BBOX;
	loogie->s.effects |= RF_FULLBRIGHT;
	VectorClear (loogie->mins);
	VectorClear (loogie->maxs);
	
	loogie->s.modelindex = gi.modelindex ("models/objects/loogy/tris.md2");
	loogie->owner = self;
	loogie->touch = loogie_touch;
	loogie->nextthink = level.time + 20;
	loogie->think = G_FreeEdict;
	loogie->dmg = damage;
	gi.linkentity (loogie);

	tr = gi.trace (self->s.origin, NULL, NULL, loogie->s.origin, loogie, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (loogie->s.origin, -10, dir, loogie->s.origin);
		loogie->touch (loogie, tr.ent, NULL, NULL);
	}

}

/*
==========
Blue Blaster
==========
*/
void fire_blueblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *bolt;
	trace_t tr;

	VectorNormalize (dir);

	bolt = G_Spawn ();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	bolt->s.modelindex = gi.modelindex ("models/objects/blaser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
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

/*
==========
Plasma (Phalanx)
==========
*/
void plasma_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BLASTER);
	}
	
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_BLASTER);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PLASMA_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	G_FreeEdict (ent);
}


// RAFAEL
void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *plasma;

	plasma = G_Spawn();
	VectorCopy (start, plasma->s.origin);
	VectorCopy (dir, plasma->movedir);
	vectoangles (dir, plasma->s.angles);
	VectorScale (dir, speed, plasma->velocity);
	plasma->movetype = MOVETYPE_FLYMISSILE;
	plasma->clipmask = MASK_SHOT;
	plasma->solid = SOLID_BBOX;

	VectorClear (plasma->mins);
	VectorClear (plasma->maxs);
	
	plasma->owner = self;
	plasma->touch = plasma_touch;
	plasma->nextthink = level.time + 8000/speed;
	plasma->think = G_FreeEdict;
	plasma->dmg = damage;
	plasma->radius_dmg = radius_damage;
	plasma->dmg_radius = damage_radius;
	plasma->s.sound = gi.soundindex ("weapons/rockfly.wav");
	
	plasma->s.modelindex = gi.modelindex ("sprites/s_photon.sp2");
	plasma->s.effects |= EF_PLASMA | EF_ANIM_ALLFAST;
	
	if (self->client)
		check_dodge (self, plasma->s.origin, dir, speed);

	gi.linkentity (plasma);

	
}

/*
========
Heat Seaking Missile
(Is now rockheat_*)
========
*/


void rockheat_think (edict_t *self)
{
	edict_t		*target = NULL;
	edict_t		*aquire = NULL;
	vec3_t		vec;
	vec3_t		oldang;
	int			len;
	int			oldlen = 0;

	VectorClear (vec);

	if (self->owner->deadflag == DEAD_DEAD)
		G_FreeEdict (self);

	// aquire new target
	while (( target = findradius (target, self->s.origin, 1024)) != NULL)
	{
		
		if (self->owner == target)
			continue;
		if (!target->svflags & SVF_MONSTER)
			continue;
		if (!target->client)
			continue;
		if (target->health <= 0)
			continue;
		if (!visible (self, target))
			continue;
			// Paril
	// Teamplay Code
	// Teamplay-By-Classes code
		if (teamplay_classes->value)
		{
			if (self->owner->client->resp.class == target->client->resp.class)
			{
				//gi.bprintf ("Class attacking each other with teamplay_classes, ignored\n");
				return;
			}
		}
	// Teamplay-By-Type code
	// Ground VS Air
	// Shark counts as air..
		if (teamplay_landtype->value)
		{
			if (self->owner->client->resp.landtype == target->client->resp.landtype)
			{
				//gi.bprintf ("Team attacking each other with teamplay_lantype, ignored\n");
				return;
			}
		}

		
		// if we need to reduce the tracking cone
		/*
		{
			vec3_t	vec;
			float	dot;
			vec3_t	forward;
	
			AngleVectors (self->s.angles, forward, NULL, NULL);
			VectorSubtract (target->s.origin, self->s.origin, vec);
			VectorNormalize (vec);
			dot = DotProduct (vec, forward);
	
			if (dot > 0.6)
				continue;
		}
		*/

		if (!infront (self, target))
			continue;

		VectorSubtract (self->s.origin, target->s.origin, vec);
		len = VectorLength (vec);

		if (aquire == NULL || len < oldlen)
		{
			aquire = target;
			self->target_ent = aquire;
			oldlen = len;
		}
	}

	if (aquire != NULL)
	{
		VectorCopy (self->s.angles, oldang);
		VectorSubtract (aquire->s.origin, self->s.origin, vec);
		
		vectoangles (vec, self->s.angles);
		
		VectorNormalize (vec);
		VectorCopy (vec, self->movedir);
		VectorScale (vec, 500, self->velocity);
	}

	self->nextthink = level.time + 0.1;
}
void ForceGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
// RAFAEL
void fire_rockheat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *heat;

	if (self->client)
	{
		if (self->client->pers.abilities.dchick_plasma)
		{
			speed = speed *= 0.80;
		}
	}

	heat = G_Spawn();
	VectorCopy (start, heat->s.origin);
	VectorCopy (dir, heat->movedir);
	vectoangles (dir, heat->s.angles);
	VectorScale (dir, speed, heat->velocity);
	heat->movetype = MOVETYPE_FLYMISSILE;
	heat->clipmask = MASK_SHOT;
	heat->solid = SOLID_BBOX;
	heat->s.effects |= EF_ROCKET;
	VectorClear (heat->mins);
	VectorClear (heat->maxs);
	heat->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	heat->owner = self;
	heat->touch = rocket_touch;

	heat->nextthink = level.time + 0.1;
	heat->think = rockheat_think;
	
	heat->dmg = damage;
	heat->radius_dmg = radius_damage;
	heat->dmg_radius = damage_radius;
	heat->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (self->client)
		check_dodge (self, heat->s.origin, dir, speed);

	gi.linkentity (heat);
}
/*
=========
Ion Ripper
=========
*/

void ionripper_sparks (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (0);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe4 + (rand()&3));
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}

// RAFAEL
void ionripper_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
			PlayerNoise (self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_BLASTER);

	}
	else
	{
		return;
	}

	G_FreeEdict (self);
}


// RAFAEL
void fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *ion;
	trace_t tr;

	VectorNormalize (dir);

	ion = G_Spawn ();
	VectorCopy (start, ion->s.origin);
	VectorCopy (start, ion->s.old_origin);
	vectoangles (dir, ion->s.angles);
	VectorScale (dir, speed, ion->velocity);

	ion->movetype = MOVETYPE_WALLBOUNCE;
	ion->clipmask = MASK_SHOT;
	ion->solid = SOLID_BBOX;
	ion->s.effects |= effect;

	ion->s.renderfx |= RF_FULLBRIGHT;

	VectorClear (ion->mins);
	VectorClear (ion->maxs);
	ion->s.modelindex = gi.modelindex ("models/objects/boomrang/tris.md2");
	ion->s.sound = gi.soundindex ("misc/lasfly.wav");
	ion->owner = self;
	ion->touch = ionripper_touch;
	ion->nextthink = level.time + 3;
	ion->think = ionripper_sparks;
	ion->dmg = damage;
	ion->dmg_radius = 100;
	gi.linkentity (ion);

	if (self->client)
		check_dodge (self, ion->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, ion->s.origin, ion, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (ion->s.origin, -10, dir, ion->s.origin);
		ion->touch (ion, tr.ent, NULL, NULL);
	}

}


/*
=================
fire_laser
test
=================
*/
void fire_laser (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
   trace_t     tr;
   vec3_t      from;
   vec3_t      end;

   VectorNormalize (dir);

      // set origin of laser beam at gun barrel.
      // note that the barrel is rotating, so the beams will 
      // originate from different places each time.
      VectorMA (start, 8192, dir, end);
      VectorCopy (start, from);
      // trace for end point of laser beam.
      // the laser aim is perfect. 
      // no random aim like the machinegun
      tr = gi.trace (from, NULL, NULL, end, self, MASK_SHOT);      
      // send laser beam temp entity to clients
      VectorCopy (tr.endpos, from);
      gi.WriteByte (svc_temp_entity);
      gi.WriteByte (TE_BFG_LASER);      
      gi.WritePosition (start);
      gi.WritePosition (tr.endpos);
      gi.multicast (self->s.origin, MULTICAST_PHS);

      
      if ((tr.ent != self) && (tr.ent->takedamage))
         T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal,
damage, 0, (DAMAGE_ENERGY|DAMAGE_NO_ARMOR), MOD_LASER);
      else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
      {  // hit a brush, send clients 
         // a light flash and sparks temp entity.
         gi.WriteByte (svc_temp_entity);
         gi.WriteByte (TE_BLOOD);
         gi.WritePosition (tr.endpos);
         gi.WriteDir (tr.plane.normal);
         gi.multicast (self->s.origin, MULTICAST_PVS);
	  gi.sound(self, CHAN_WEAPON, gi.soundindex ("weapons/burn.wav"), 1, ATTN_NORM, 0);
      }

}	


// Rogue stuff

/*
=================
fire_blaster2

Fires a single green blaster bolt.  Used by monsters, generally.
=================
*/
void blaster2_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	int		damagestat;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		// the only time players will be firing blaster2 bolts will be from the 
		// defender sphere.
		mod = MOD_BLASTER;

		if (self->owner)
		{
			damagestat = self->owner->takedamage;
			self->owner->takedamage = DAMAGE_NO;
			if (self->dmg >= 5)
				T_RadiusDamage(self, self->owner, self->dmg*3, other, self->dmg_radius, 0);
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
			self->owner->takedamage = damagestat;
		}
		else
		{
			if (self->dmg >= 5)
				T_RadiusDamage(self, self->owner, self->dmg*3, other, self->dmg_radius, 0);
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
		}
	}
	else
	{
		//PMM - yeowch this will get expensive
		if (self->dmg >= 5)
			T_RadiusDamage(self, self->owner, self->dmg*3, self->owner, self->dmg_radius, 0);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER2);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE; // special net code is used for projectiles
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles2 (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	
		if (effect)
			bolt->s.effects |= EF_TRACKER;
		bolt->dmg_radius = 128;
		bolt->s.modelindex = gi.modelindex ("models/proj/laser2/tris.md2");
		bolt->touch = blaster2_touch;

	bolt->owner = self;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
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

void fire_blaster2_homing (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE; // special net code is used for projectiles
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles2 (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	
		if (effect)
			bolt->s.effects |= EF_TRACKER;
		bolt->dmg_radius = 128;
		bolt->s.modelindex = gi.modelindex ("models/proj/laser2/tris.md2");
		bolt->touch = blaster2_touch;

	bolt->owner = self;
	bolt->nextthink = level.time + 0.1;
	bolt->think = rockheat_think;
	bolt->dmg = damage;
	bolt->classname = "bolt";
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


static void fire_beams (edict_t *self, vec3_t start, vec3_t aimdir, vec3_t offset, int damage, int kick, int te_beam, int te_impact, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	vec3_t		water_start, endpoint;
	qboolean	water = false, underwater = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	vec3_t		beam_endpt;

//	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
//	if (!(tr.fraction < 1.0))
//	{
	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	VectorMA (start, 8192, forward, end);

	if (gi.pointcontents (start) & MASK_WATER)
	{
//		gi.dprintf ("Heat beam under water\n");
		underwater = true;
		VectorCopy (start, water_start);
		content_mask &= ~MASK_WATER;
	}

	tr = gi.trace (start, NULL, NULL, end, self, content_mask);

	// see if we hit water
	if (tr.contents & MASK_WATER)
	{
		water = true;
		VectorCopy (tr.endpos, water_start);

		if (!VectorCompare (start, tr.endpos))
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_HEATBEAM_SPARKS);
//			gi.WriteByte (50);
			gi.WritePosition (water_start);
			gi.WriteDir (tr.plane.normal);
//			gi.WriteByte (8);
//			gi.WriteShort (60);
			gi.multicast (tr.endpos, MULTICAST_PVS);
		}
		// re-trace ignoring water this time
		tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
	}
	VectorCopy (tr.endpos, endpoint);
//	}

	// halve the damage if target underwater
	if (water)
	{
		damage = damage /2;
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_ENERGY, mod);
			}
			else
			{
				if ((!water) && (strncmp (tr.surface->name, "sky", 3)))
				{
					// This is the truncated steam entry - uses 1+1+2 extra bytes of data
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_HEATBEAM_STEAM);
//					gi.WriteByte (20);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
//					gi.WriteByte (0xe0);
//					gi.WriteShort (60);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if ((water) || (underwater))
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL2);
//		gi.WriteByte (8);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}

	if ((!underwater) && (!water))
	{
		VectorCopy (tr.endpos, beam_endpt);
	}
	else
	{
		VectorCopy (endpoint, beam_endpt);
	}
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (te_beam);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (beam_endpt);
	gi.multicast (self->s.origin, MULTICAST_ALL);

}


/*
=================
fire_heat

Fires a single heat beam.  Zap.
=================
*/
void fire_heat (edict_t *self, vec3_t start, vec3_t aimdir, vec3_t offset, int damage, int kick, qboolean monster)
{
	if (monster)
		fire_beams (self, start, aimdir, offset, damage, kick, TE_MONSTER_HEATBEAM, TE_HEATBEAM_SPARKS, MOD_BLASTER);
	else
		fire_beams (self, start, aimdir, offset, damage, kick, TE_HEATBEAM, TE_HEATBEAM_SPARKS, MOD_BLASTER);
}

#define TRACKER_DAMAGE_FLAGS	(DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK)
#define TRACKER_IMPACT_FLAGS	(DAMAGE_ENERGY)

#define TRACKER_DAMAGE_TIME		0.5		// seconds

void tracker_pain_daemon_think (edict_t *self)
{
	static vec3_t	pain_normal = { 0, 0, 1 };
	int				hurt;

	if(!self->inuse)
		return;

	if((level.time - self->timestamp) > TRACKER_DAMAGE_TIME)
	{
		if(!self->enemy->client)
			self->enemy->s.effects &= ~EF_TRACKERTRAIL;
		G_FreeEdict (self);
	}
	else
	{
		if(self->enemy->health > 0)
		{
//			gi.dprintf("ouch %x\n", self);
			T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, pain_normal,
						self->dmg, 0, TRACKER_DAMAGE_FLAGS, MOD_BLASTER);
			
			// if we kill the player, we'll be removed.
			if(self->inuse)
			{
				// if we killed a monster, gib them.
				if (self->enemy->health < 1)
				{
					if(self->enemy->gib_health)
						hurt = - self->enemy->gib_health;
					else
						hurt = 500;

//					gi.dprintf("non-player killed. ensuring gib!  %d\n", hurt);
					T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin,
								pain_normal, hurt, 0, TRACKER_DAMAGE_FLAGS, MOD_BLASTER);
				}

				if(self->enemy->client)
					self->enemy->client->tracker_pain_framenum = level.framenum + 1;
				else
					self->enemy->s.effects |= EF_TRACKERTRAIL;
				
				self->nextthink = level.time + FRAMETIME;
			}
		}
		else
		{
			if(!self->enemy->client)
				self->enemy->s.effects &= ~EF_TRACKERTRAIL;
			G_FreeEdict (self);
		}
	}
}

void tracker_pain_daemon_spawn (edict_t *owner, edict_t *enemy, int damage)
{
	edict_t	 *daemon;

	if(enemy == NULL)
		return;

	daemon = G_Spawn();
	daemon->classname = "pain daemon";
	daemon->think = tracker_pain_daemon_think;
	daemon->nextthink = level.time + FRAMETIME;
	daemon->timestamp = level.time;
	daemon->owner = owner;
	daemon->enemy = enemy;
	daemon->dmg = damage;
}

void tracker_explode (edict_t *self, cplane_t *plane)
{
	vec3_t	dir;

	if(!plane)
		VectorClear (dir);
	else
		VectorScale (plane->normal, 256, dir);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

//	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/disrupthit.wav"), 1, ATTN_NORM, 0);
//	tracker_boom_spawn(self->s.origin);

	G_FreeEdict (self);
}

void tracker_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	damagetime;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if((other->svflags & SVF_MONSTER) || other->client)
		{
			if(other->health > 0)		// knockback only for living creatures
			{
				// PMM - kickback was times 4 .. reduced to 3
				// now this does no damage, just knockback
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
							/* self->dmg */ 0, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_BLASTER);
				
				if (!(other->flags & (FL_FLY|FL_SWIM)))
					other->velocity[2] += 140;
				
				damagetime = ((float)self->dmg)*FRAMETIME;
				damagetime = damagetime / TRACKER_DAMAGE_TIME;
//				gi.dprintf ("damage is %f\n", damagetime);

				tracker_pain_daemon_spawn (self->owner, other, (int)damagetime);
			}
			else						// lots of damage (almost autogib) for dead bodies
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
							self->dmg*4, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_BLASTER);
			}
		}
		else	// full damage in one shot for inanimate objects
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
						self->dmg, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_BLASTER);
		}
	}

	tracker_explode (self, plane);
	return;
}

void tracker_fly (edict_t *self)
{
	vec3_t	dest;
	vec3_t	dir;
	vec3_t	center;

	if ((!self->enemy) || (!self->enemy->inuse) || (self->enemy->health < 1))
	{
		tracker_explode (self, NULL);
		return;
	}
/*
	VectorCopy (self->enemy->s.origin, dest);
	if(self->enemy->client)
		dest[2] += self->enemy->viewheight;
*/
	// PMM - try to hunt for center of enemy, if possible and not client
	if(self->enemy->client)
	{
		VectorCopy (self->enemy->s.origin, dest);
		dest[2] += self->enemy->viewheight;
	}
	// paranoia
	else if (VectorCompare(self->enemy->absmin, vec3_origin) || VectorCompare(self->enemy->absmax, vec3_origin))
	{
		VectorCopy (self->enemy->s.origin, dest);
	}
	else
	{
		VectorMA (vec3_origin, 0.5, self->enemy->absmin, center);
		VectorMA (center, 0.5, self->enemy->absmax, center);
		VectorCopy (center, dest);
	}

	VectorSubtract (dest, self->s.origin, dir);
	VectorNormalize (dir);
	vectoangles2 (dir, self->s.angles);
	VectorScale (dir, self->speed, self->velocity);
	VectorCopy(dest, self->monsterinfo.saved_goal);

	self->nextthink = level.time + 0.1;
}

void fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles2 (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->speed = speed;
	bolt->s.effects = EF_TRACKER;
	bolt->s.sound = gi.soundindex ("weapons/disrupt.wav");
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	
	bolt->s.modelindex = gi.modelindex ("models/proj/disintegrator/tris.md2");
	bolt->touch = tracker_touch;
	bolt->enemy = enemy;
	bolt->owner = self;
	bolt->dmg = damage;
	bolt->classname = "tracker";
	gi.linkentity (bolt);

	if(enemy)
	{
		bolt->nextthink = level.time + 0.1;
		bolt->think = tracker_fly;
	}
	else
	{
		bolt->nextthink = level.time + 10;
		bolt->think = G_FreeEdict;
	}

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

//Fixbot Weld
//Based on fire_player_melee
void fire_weld (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;
	int r;

	r = rndnum (0, 4);

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);

	if (self->client->resp.next_weld_sound < level.time)
	{
		gi.sound (self, CHAN_AUTO, gi.soundindex(va("misc/welder%i.wav", r)), 1, ATTN_NORM, 0);
		self->client->resp.next_weld_sound=level.time+0.30;
	}


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (tr.endpos);
	gi.WriteDir (point);
	gi.multicast (tr.endpos, MULTICAST_PVS);

	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_ARMOR, mod);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);
	}
}


/*
=================
fire_rocket
=================
*/
void clust_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;
             //Sean added these 4 vectors
 
             vec3_t   grenade1;
             vec3_t   grenade2;
             vec3_t   grenade3;
             vec3_t   grenade4;
             vec3_t   grenade5;
             vec3_t   grenade6;
             vec3_t   grenade7;
             vec3_t   grenade8;
             vec3_t   grenade9;
             vec3_t   grenade10;
             vec3_t   grenade11;
             vec3_t   grenade12;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				//while(n--)
//					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

             // SumFuka did this bit : give grenades up/outwards velocities
	// Vectorset (item, pitch (updown), yaw (leftright), roll (inout);
             VectorSet(grenade1,20,20,40);
             VectorSet(grenade2,20,-20,40);
             VectorSet(grenade3,-20,20,40);
             VectorSet(grenade4,-20,-20,40);
             VectorSet(grenade5,10,10,20);
             VectorSet(grenade6,10,-10,20);
             VectorSet(grenade7,-10,10,20);
             VectorSet(grenade8,-10,-10,20);
             VectorSet(grenade9,40,40,80);
             VectorSet(grenade10,40,-40,80);
             VectorSet(grenade11,-40,40,80);
             VectorSet(grenade12,-40,-40,80);
             // Sean : explode the four grenades outwards
             fire_grenade(ent, origin, grenade1, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade2, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade3, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade4, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade5, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade6, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade7, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade8, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade9, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade10, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade11, 120, 10, 3.0, 120);
             fire_grenade(ent, origin, grenade12, 120, 10, 3.0, 120);

	G_FreeEdict (ent);
}

void fire_clust (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_GRENADE;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = clust_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


void ProxGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

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

void ProxGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

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
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

// CCH: New think function for proximity grenades
void proxim_think (edict_t *ent)
{
 edict_t *blip = NULL;

 if (level.time > ent->delay)
 {
		ent->owner->client->pers.abilities.brain_proxies --;
        Grenade_Explode(ent);
        return;
 }
 
 ent->think = proxim_think;
 while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
 {
        if (!(blip->svflags & SVF_MONSTER) && !blip->client)
                continue;
        if (blip == ent->owner)
                continue;
        if (!blip->takedamage)
                continue;
        if (blip->health <= 0)
                continue;
        if (!visible(ent, blip))
                continue;

		ent->owner->client->pers.abilities.brain_proxies --;
        ent->think = Grenade_Explode;
        break;
 }

 ent->nextthink = level.time + .1;
}


void fire_proxgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;

	// CCH: use our proximity think instead
	grenade->nextthink = level.time + .1;
	grenade->think = proxim_think;
	grenade->delay = level.time + 60;


	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void tip_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	//if (surf && (surf->flags & SURF_SKY))
	//{
		//G_FreeEdict (self);
		//return;
	//}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
		G_FreeEdict (self);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

}

void fire_tip (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int which_mod)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_MORE_BOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= 0;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/monsters/parasite/tip/tris.md2");
//	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = tip_touch;
	bolt->nextthink = level.time + 30;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "parasitething";
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


/*
=================
Lightning Bolts
=================
*/

void lightningsmall_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				//while(n--)
//					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	//T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

/*	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);*/

	G_FreeEdict (ent);
}

void fire_lightningsmall (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_FLAG2;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/proj/lightning/tris.md2");
	rocket->owner = self;
	rocket->touch = lightningsmall_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "light_small";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void lightningbig_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;
             //Sean added these 4 vectors
 
             vec3_t   grenade1;
             vec3_t   grenade2;
             vec3_t   grenade3;
             vec3_t   grenade4;
             vec3_t   grenade5;
             vec3_t   grenade6;
             vec3_t   grenade7;
             vec3_t   grenade8;
             vec3_t   grenade9;
             vec3_t   grenade10;
             vec3_t   grenade11;
             vec3_t   grenade12;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
			//	while(n--)
//					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	/*gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);*/

             // SumFuka did this bit : give grenades up/outwards velocities
	// Vectorset (item, pitch (updown), yaw (leftright), roll (inout);
             VectorSet(grenade1,	20,	20,40);
             VectorSet(grenade2,	20,	-30,40);
             VectorSet(grenade3,	40,	30,40);
             VectorSet(grenade4,	40,	-20,40);
             VectorSet(grenade5,	30,	10,20);
             VectorSet(grenade6,	30,	-10,20);
             VectorSet(grenade7,	10,	40,20);
             VectorSet(grenade8,	10,	-10,20);
             VectorSet(grenade9,	50,	80,80);
             VectorSet(grenade10,	45,	-40,80);
             VectorSet(grenade11,	23,	60,80);
             VectorSet(grenade12,	19,	-40,80);
             // Sean : explode the four grenades outwards
             fire_lightningsmall(ent, origin,	grenade1,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade2,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade3,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade4,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade5,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade6,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade7,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade8,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,	grenade9,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,  grenade10,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,  grenade11,	80, 100, 100, 120);
             fire_lightningsmall(ent, origin,  grenade12,	80, 100, 100, 120);

	G_FreeEdict (ent);
}

void fire_lightning_bigbolt (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_FLAG2;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/proj/lightning/tris.md2");
	rocket->owner = self;
	rocket->touch = lightningbig_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "light_big";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void fire_player_melee_bbrain_suck (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// do the damage
		// FIXME - make the damage appear at right spot and direction
		T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);
		//gi.sound (self, CHAN_WEAPON, gi.soundindex ("infantry/melee2.wav"), 1, ATTN_NORM, 0);
		self->health += damage;

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);
	}
}

// Ability
// BFG Ball Of Doometh
void Ball_Explode (edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->owner))
				continue;

			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void Ball_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	//if (surf && (surf->flags & SURF_SKY))
	//{
		//G_FreeEdict (self);
		//return;
	//}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (!other->takedamage)
		return;

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
	T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
void Ball_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	Ball_Explode (self);
}
void Ball_Think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;
	edict_t		*target = NULL;
	edict_t		*aquire = NULL;
	vec3_t		vec;
	vec3_t		oldang;
	int			len;
	int			oldlen = 0;

	VectorClear (vec);

	if (self->owner->deadflag == DEAD_DEAD)
		G_FreeEdict (self);

	// aquire new target
	while (( target = findradius (target, self->s.origin, 1024)) != NULL)
	{
		
		if (self->owner == target)
			continue;
		if (!target->svflags & SVF_MONSTER)
			continue;
		if (!target->client)
			continue;
		if (target->health <= 0)
			continue;
		if (!visible (self, target))
			continue;
			// Paril
	// Teamplay Code
	// Teamplay-By-Classes code
	if (teamplay_classes->value)
	{
		if (self->owner->client->resp.class == target->client->resp.class)
		{
			//gi.bprintf ("Class attacking each other with teamplay_classes, ignored\n");
			return;
		}
	}
	// Teamplay-By-Type code
	// Ground VS Air
	// Shark counts as air..
	if (teamplay_landtype->value)
	{
		if (self->owner->client->resp.landtype == target->client->resp.landtype)
		{
			//gi.bprintf ("Team attacking each other with teamplay_lantype, ignored\n");
			return;
		}
	}

		
		// if we need to reduce the tracking cone
		/*
		{
			vec3_t	vec;
			float	dot;
			vec3_t	forward;
	
			AngleVectors (self->s.angles, forward, NULL, NULL);
			VectorSubtract (target->s.origin, self->s.origin, vec);
			VectorNormalize (vec);
			dot = DotProduct (vec, forward);
	
			if (dot > 0.6)
				continue;
		}
		*/

		if (!infront (self, target))
			continue;

		VectorSubtract (self->s.origin, target->s.origin, vec);
		len = VectorLength (vec);

		if (aquire == NULL || len < oldlen)
		{
			aquire = target;
			self->target_ent = aquire;
			oldlen = len;
		}
	}

	if (aquire != NULL)
	{
		VectorCopy (self->s.angles, oldang);
		VectorSubtract (aquire->s.origin, self->s.origin, vec);
		
		vectoangles (vec, self->s.angles);
		
		VectorNormalize (vec);
		VectorCopy (vec, self->movedir);
		VectorScale (vec, 500, self->velocity);
	}

	self->nextthink = level.time + 0.1;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	if (self->owner->health < 1)
		Ball_Explode (self);

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
			{
				self->speed = 600;
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);
			}

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
	if (++self->s.frame < 9)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void Launch_Ball (edict_t *self, vec3_t start, vec3_t dir, int damage, float damage_radius)
{
	edict_t	*bfg;
	int speed;

	speed = 600;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	/*bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;*/
	bfg->s.frame = 0;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("models/objects/bfgball/tris.md2"/*"sprites/s_bfg1.sp2"*/);
	bfg->owner = self;
	bfg->touch = Ball_Touch;
	bfg->nextthink2 = level.time + 8;
	bfg->think2 = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "ball of doom";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	bfg->think = Bfg_Think;
	bfg->nextthink = level.time + 0.1;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}
void fire_fire (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);

void Fire_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	mod = MOD_BLASTER;
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void Fire_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t   grenade1;
	
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
		if (crandom() < .4)
		{
			VectorSet(grenade1,20,20,crandom()*40+30);
			fire_fire(ent->owner, ent->s.origin, grenade1, 90, 5, crandom()*2, 120,false);
			if (crandom() < .2)
				Fire_Explode (ent);
		}
	

		return;

	ent->enemy = other;
	Fire_Explode (ent);
}

void FCluster_Explode (edict_t *ent)

{
	vec3_t		origin;

	//Sean added these 4 vectors

	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;
	vec3_t   grenade5;
	vec3_t   grenade6;
	vec3_t   grenade7;
	vec3_t   grenade8;
	int mod = MOD_BLASTER;

	//if (ent->owner->client)
		//PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// SumFuka did this bit : give grenades up/outwards velocities
	VectorSet(grenade1,20,20,crandom()*20+30);
	VectorSet(grenade2,20,-20,crandom()*20+30);
	VectorSet(grenade3,-20,20,crandom()*20+30);
	VectorSet(grenade4,-20,-20,crandom()*20+30);
	VectorSet(grenade5,30,30,crandom()*20+40);
	VectorSet(grenade6,30,-30,crandom()*20+40);
	VectorSet(grenade7,-30,30,crandom()*20+40);
	VectorSet(grenade8,-30,-30,crandom()*20+40);
	


	// Sean : explode the four grenades outwards
	fire_fire(ent->owner, origin, grenade1, 90, 11, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade2, 90, 10, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade3, 94, 12, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade4, 95, 10, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade5, 90, 11, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade6, 90, 10, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade7, 94, 12, crandom()*4+1, 120,false);
	fire_fire(ent->owner, origin, grenade8, 95, 10, crandom()*4+1, 120,false);

	 G_FreeEdict (ent);
}
void FireGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		FCluster_Explode (ent);
	}

	ent->enemy = other;
	FCluster_Explode (ent);
	G_FreeEdict (ent);
}
//**********************
//  FIRE RADII DAMAGE
//**********************

void Fire_Burn_U (edict_t *ent)
{
	if (ent->waterlevel)
		ent->think2 = G_FreeEdict;

	T_RadiusDamage(ent,ent->owner,50,NULL,100,MOD_HG_SPLASH);
	ent->nextthink2 = level.time + crandom();
}

void fire_fire (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects = EF_FLAG1; //EF_GRENADE | EF_BLASTER;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/fire/tris.md2");
	grenade->owner = self;
	grenade->touch = Fire_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Fire_Explode;
	grenade->nextthink2 = level.time + .3;
	grenade->think2 = Fire_Burn_U;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "fire";

	if (timer <= 0.0)
		Fire_Explode (grenade);
	else
	{
		gi.linkentity (grenade);
	}
}


void Launch_BigBomb (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_TOSS;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET | EF_BLASTER | EF_GRENADE;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = FireGrenade_Touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

/*
=================
fire_instlaser

Instant hit laser, for the
Death Tank.
=================
*/
void fire_instlaser (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
				(tr.ent->solid == SOLID_BBOX))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

/*
=================
fire_phlas
=================
*/

void phlas_expl (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 3)
		self->think = G_FreeEdict;
}
void phlas_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}


	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/shootdplas.wav"), 1, ATTN_NORM, 0);
	ent->solid = SOLID_NOT;
	ent->touch = NULL;
	VectorMA (ent->s.origin, -1 * FRAMETIME, ent->velocity, ent->s.origin);
	VectorClear (ent->velocity);
	ent->s.modelindex = gi.modelindex ("sprites/s_bfg2.sp2");
	ent->s.frame = 0;
	ent->s.sound = 0;
	ent->s.effects &= ~EF_ANIM_ALLFAST;
	ent->think = phlas_expl;
	ent->nextthink = level.time + FRAMETIME;
	ent->enemy = other;

	G_FreeEdict (ent);
}

void fire_phlas (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.modelindex = gi.modelindex ("sprites/s_photon.sp2");
	rocket->s.effects |= EF_BLASTER | EF_PLASMA | EF_ANIM_ALLFAST;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = self;
	rocket->touch = phlas_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/shootdplas.wav");
	rocket->classname = "phlas";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void explogib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	int i;

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
		mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		if (self->waterlevel)
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	T_RadiusDamage (self, self->owner, 150, NULL, 300, MOD_R_SPLASH);
	for (i = 0; i<6; i++)
	{
		//ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2");
		ThrowDebris (self, "models/objects/gibs/sm_meat/tris.md2", 4, self->s.origin, self->owner);
	}

	G_FreeEdict (self);
}

void fire_explogib (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int which_mod)
{
	edict_t	*bolt;
	trace_t	tr;
	char *gib;
	int r;

	speed *= 2;

	gib = "models/objects/gibs/chest/tris.md2";

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE; // special net code is used for projectiles
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_TOSS;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex (gib);
	bolt->owner = self;
	bolt->touch = explogib_touch;
	bolt->nextthink = level.time + 12;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "explogib";
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





//New functions by Gregg Reno- used for testing freeze on self
void freeze_player(edict_t *ent)
{
    if (ent->health <= 0)//5/99
        return;

	// make em frozen
	ent->frozen = 1;

	// set the time till thaw
	ent->frozentime = level.time + 4;	//4 seconds of freeze (was 6)
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/freezer/freeze.wav"), 1, ATTN_NORM, 0);

}

void unfreeze_player(edict_t *ent)
{
	ent->frozen = 0;
	ent->frozentime = level.time - 1;
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/freezer/shatter1.wav"), 1, ATTN_NORM, 0);
}

/*
================================================
 Same as blaster_touch with some freezing info 
=================================================
*/
void freezer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{ 
    int mod;
	

      mod = MOD_BLASTER;
//note; returns in this part of the code will produce a richocet type effect,try shooting bots
	if (other == self->owner)
		return;
//	if (other->bot_client)
//		return;

    if (other->client)
	{
		if (ctf->value && other->client->resp.ctf_team == self->owner->client->resp.ctf_team)
		{
			other->frozen = 0;
		}
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}
/*note: Probably better to ignore feigns because of returns in 
        ClientThink, but you still take damage*/
	if (other->client && !(other->frozen))
	{
		if (ctf->value)
		{
			if (other->client->resp.ctf_team != self->owner->client->resp.ctf_team)
				freeze_player(other);
		}
		else
			freeze_player(other);
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
		
		T_Damage (other, self, self->owner, self->velocity,
		self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_freezer (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*freezer;
	trace_t	tr;

	VectorNormalize (dir);

	freezer = G_Spawn();
	VectorCopy (start, freezer->s.origin);
	VectorCopy (start, freezer->s.old_origin);
	vectoangles (dir, freezer->s.angles);
	VectorScale (dir, speed, freezer->velocity);
	freezer->movetype = MOVETYPE_FLYMISSILE;
	freezer->clipmask = MASK_SHOT;
	freezer->solid = SOLID_BBOX;
	freezer->s.effects |= effect |EF_COLOR_SHELL;
	freezer->s.renderfx |= RF_SHELL_BLUE|RF_SHELL_GREEN|RF_SHELL_RED;
	VectorClear (freezer->mins);
	VectorClear (freezer->maxs);
	freezer->s.modelindex = gi.modelindex ("models/objects/debris2/tris.md2");
	freezer->classname = "iceball";
	freezer->owner = self;
	freezer->touch = freezer_touch;
	freezer->nextthink = level.time + 2;
	freezer->think = G_FreeEdict;
	freezer->dmg = damage;
	gi.linkentity (freezer);

	if (self->client)
		check_dodge (self, freezer->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, freezer->s.origin, freezer, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (freezer->s.origin, -10, dir, freezer->s.origin);
		freezer->touch (freezer, tr.ent, NULL, NULL);
	}
}




/*
========================
fire_flechette
========================
*/
void flechette_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		dir;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
//gi.dprintf("t_damage %s\n", other->classname);
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
			self->dmg, self->dmg_radius, 0, MOD_BLASTER);
	}
	else
	{
		if(!plane)
			VectorClear (dir);
		else
			VectorScale (plane->normal, 256, dir);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		gi.WriteDir (dir);
		gi.multicast (self->s.origin, MULTICAST_PVS);

//		T_RadiusDamage(self, self->owner, 24, self, 48, MOD_ETF_RIFLE);
	}

	G_FreeEdict (self);
}

void fire_flechette (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int kick)
{
	edict_t *flechette;

	VectorNormalize (dir);

	flechette = G_Spawn();
	VectorCopy (start, flechette->s.origin);
	VectorCopy (start, flechette->s.old_origin);
	vectoangles2 (dir, flechette->s.angles);

	VectorScale (dir, speed, flechette->velocity);
	flechette->movetype = MOVETYPE_FLYMISSILE;
	flechette->clipmask = MASK_SHOT;
	flechette->solid = SOLID_BBOX;
	flechette->s.renderfx = RF_FULLBRIGHT;
	VectorClear (flechette->mins);
	VectorClear (flechette->maxs);
	
	flechette->s.modelindex = gi.modelindex ("models/proj/flechette/tris.md2");

//	flechette->s.sound = gi.soundindex ("");			// FIXME - correct sound!
	flechette->owner = self;
	flechette->touch = flechette_touch;
	flechette->nextthink = level.time + 8000/speed;
	flechette->think = G_FreeEdict;
	flechette->dmg = damage;
	flechette->dmg_radius = kick;

	gi.linkentity (flechette);
	
	if (self->client)
		check_dodge (self, flechette->s.origin, dir, speed);
}