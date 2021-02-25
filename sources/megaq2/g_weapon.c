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
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
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
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
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

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
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
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;

	bolt->s.effects |= EF_COLOR_SHELL;
	bolt->s.renderfx |= (RF_SHELL_GREEN|RF_SHELL_RED);

	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/flash/tris.md2");
	//bolt->s.modelindex = gi.modelindex ("models/objects/needle/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (hyper)
		bolt->spawnflags = 1;
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
	//grenade->s.modelindex = gi.modelindex ("models/objects/crashb/tris.md2");
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
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
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
	//rocket->s.modelindex = gi.modelindex ("models/objects/crashb/tris.md2");
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
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
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
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
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
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000/speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}




//peewee start
//universal remove shot function
void remove_shot(edict_t *ent)
{
	//if owner is a client decrement shots
	if (ent->owner->client)
		ent->owner->client->pers.shot_limit--;

	//remove object
	G_FreeEdict(ent);
}







/*
=================
X-Buster
=================
*/
void xbuster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_BUSTER);
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

	//if count was true
	if (self->radius_dmg == 1)
		G_FreeEdict(self); //simply destroy ent
	else
		remove_shot(self); //decrement shot and remove object
}

void cblaster_pass (edict_t *ent)
{
	vec3_t	v;
	VectorCopy(ent->cutterangle, v);

	VectorNormalize(v);

	//restore original velocity
	vectoangles(v, ent->s.angles);
	VectorCopy(v, ent->movedir);
	VectorScale(v, 700, ent->velocity);

		//if count was true
	if (ent->radius_dmg == 1)
		ent->think = G_FreeEdict; //simply destroy ent
	else
		ent->think = remove_shot;
	ent->nextthink = level.time + 1;
}

void cblaster_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	int			rdam, damr;

	rdam = 45;
	damr = 45;

	if (other == ent->owner)
		return;

	if (other->takedamage)
	{
		int newdmg;
		newdmg = ent->dmg - other->health; //calculate new damage by subtracting other health from damage

		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BUSTER);

		ent->dmg = newdmg; //we did the damage so change the damage for next touch

		if (ent->dmg > 0) //if there still is any dmg left then don't destroy bolt
		{
			ent->think = cblaster_pass;
			ent->nextthink = level.time + 0.01;

//			VectorCopy(ent->cutterangle, ent->velocity);

			//return so it doesnt get destroyed
			return;
		}
	}



	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

//	T_RadiusDamage(ent, ent->owner, rdam, other, damr, MOD_BLASTER);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if (ent->radius_dmg == 1)
		G_FreeEdict(ent); //simply destroy ent
	else
		remove_shot(ent); //decrement shot and remove object
}

void sblaster_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	int			rdam, damr;

	rdam = 80;
	damr = 100;

	if (other == ent->owner)
		return;

	if (other->takedamage)
	{
		int newdmg;
		newdmg = ent->dmg - other->health; //calculate new damage by subtracting other health from damage

		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_SABRE_BLAST);

		ent->dmg = newdmg; //we did the damage so change the damage for next touch

		if (ent->dmg > 0) //if there still is any dmg left then don't destroy bolt
		{
			ent->think = cblaster_pass;
			ent->nextthink = level.time + 0.01;

			//return so it doesnt get destroyed
			return;
		}
	}



	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	T_RadiusDamage(ent, ent->owner, rdam, other, damr, MOD_SABRE_BLAST);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PLAIN_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	//if count was true
	if (ent->radius_dmg == 1)
		G_FreeEdict(ent); //simply destroy ent
	else
		remove_shot(ent); //decrement shot and remove object
}


void fire_cblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int type, qboolean count)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);

	//copy beginning velocity to temp variable in case we need to continue moving
	VectorCopy(dir, bolt->cutterangle);

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/plasma/tris.md2");
	bolt->touch = xbuster_touch;
	bolt->s.effects |= EF_COLOR_SHELL;

	if (type == 0)
	{
		bolt->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN);
	}
	else if (type == 1)
	{
		VectorSet(bolt->mins, -2, -2, -2);
		VectorSet(bolt->maxs, 2, 2, 2);
		bolt->s.effects |= EF_BLASTER;
		bolt->s.modelindex = gi.modelindex ("models/objects/plasma2/tris.md2");
		bolt->s.renderfx |= (RF_SHELL_GREEN);
	}
	else if (type == -1)
	{
		VectorSet(bolt->mins, -8, -8, -8);
		VectorSet(bolt->maxs, 8, 8, 8);

		bolt->s.modelindex = gi.modelindex ("models/objects/sblast/tris.md2");
		bolt->s.effects = EF_BLASTER;

		bolt->touch = sblaster_touch;
	}
	else
	{
		VectorSet(bolt->mins, -8, -8, -8);
		VectorSet(bolt->maxs, 8, 8, 8);

		bolt->s.effects |= EF_GRENADE;
		bolt->s.modelindex = gi.modelindex ("models/objects/plasma3/tris.md2");
		bolt->s.renderfx |= (RF_SHELL_BLUE);
		bolt->touch = cblaster_touch;
	}

	//if we decided not to count (blaster2 or tank)
	if (!count)
	{
		bolt->radius_dmg = 1; //mark as not counting
		bolt->think = G_FreeEdict; //set think to only remove without counting down
	}
	else
	{
		bolt->radius_dmg = 0; //mark as counting
		bolt->think = remove_shot; //set think to remove shot and count down
	}

	bolt->owner = self;
	bolt->nextthink = level.time + 3;

	//for plasma type 3 damage will be used as health
	bolt->dmg = damage;

	bolt->classname = "bolt";
	gi.linkentity (bolt);


	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}





/*
=================
armor explode - super armor
=================
*/
void armor_explode (edict_t *ent)
{
	vec3_t		origin;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	T_RadiusDamage(ent, ent, 450, NULL, 450, MOD_SARMOR);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
}


//------------------------------
// CUTTER
//------------------------------
void cutter_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void cutter_think (edict_t *ent)
{
	//decrement time
	ent->health--;

	//if time runs out
	if (ent->health == 0)
	{
		//decrement shot and remove object
		remove_shot(ent);
		return; //stop thinking
	}
	else //otherwise
		ent->nextthink = level.time + 0.1; //keep thinking


	//dont start spin for 0.4 seconds
	if (ent->health < 26)
	{
		//add spin
		vec3_t	v, f;

		v[PITCH] = ent->cutterangle[PITCH];

		//if a player shot this
		if (ent->owner && ent->owner->client)
		{
			//adjust curve for left hand
			if (ent->owner->client->pers.hand == LEFT_HANDED)
				v[YAW]   = ent->cutterangle[YAW] -= 25;
			else
				v[YAW]   = ent->cutterangle[YAW] += 25;
		}
		else //otherwise use the same spin
			v[YAW]   = ent->cutterangle[YAW] += 25;


		v[ROLL]  = ent->cutterangle[ROLL];
		AngleVectors (v, f, NULL, NULL);

		VectorScale(f, 650, f);
		VectorCopy(f, ent->velocity);
		VectorCopy(v, ent->cutterangle);
	}



	//if we are currently not solid
	if (ent->count)
	{
		//decrement timer
		ent->count--;

		//if timer is zero
		if (ent->count == 0)
		{
			//make it touchable again
			ent->solid = SOLID_BBOX;
			ent->touch = cutter_touch;
		}
	}
}

void cutter_pass (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	return;
}


void cutter_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (other->takedamage)
	{
		//we hit someone so do damage
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_CUTTER);

		//make cutter not solid so that it will pass through players
		ent->solid = SOLID_NOT;
		ent->touch = cutter_pass;

		//tell cutter think that we are nonsolid by starting counter
		ent->count = 3;

		//cutter hit so start spinning
		ent->nextthink = level.time;

	}
	else
	{
		//decrement shot and remove object
		remove_shot(ent);
	}
}

void fire_cutter (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/cutter/tris.md2");
	bolt->touch = cutter_touch;

	bolt->owner = self;

	bolt->count = 0; //clear notsolid counter
	bolt->health = 30; //initialize health timer
	bolt->nextthink = level.time + 0.1;
	bolt->think = cutter_think;

	bolt->s.sound = gi.soundindex ("weapons/cutter.wav");


	bolt->dmg = damage;
	bolt->classname = "bolt";

	//copy forward before adding spin
	VectorCopy(bolt->s.angles, bolt->cutterangle);
	//give cutter some spin
	VectorSet(bolt->avelocity,0, -2000 ,0);

	gi.linkentity (bolt);


	trace_t	tr;
	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}






//------------------
//cutter box
//------------------

void cutterbox_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (other->takedamage)
	{
		//we hit someone so do damage
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BLASTER);
	}

	G_FreeEdict(ent);
}

void fire_cutterbox (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;

	VectorNormalize (dir);


	//VectorCopy (start, grenade->s.origin);
	//VectorScale (aimdir, speed, grenade->velocity);
	//VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	//VectorSet (grenade->avelocity, 300, 300, 300);


	bolt = G_Spawn();

	bolt->svflags = SVF_DEADMONSTER;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);

	//add upward velocity
	vec3_t	forward, right, up;
	AngleVectors (dir, forward, right, up);
	VectorMA (bolt->velocity, 350, up, bolt->velocity);

	bolt->movetype = MOVETYPE_BOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/cutter/tris.md2");
	bolt->touch = cutter_touch;

	//give it spin
	VectorSet(bolt->avelocity,0, -2000 ,0);

	bolt->think = G_FreeEdict;
	bolt->nextthink = level.time + 1;

	bolt->owner = self;
	bolt->dmg = damage;
	bolt->classname = "bolt";

	gi.linkentity (bolt);
}









/*
=================
bomb man
Hyper Bombs
=================
*/

void Bomb_Exp_Explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//do damage
	T_RadiusDamage(ent, ent->owner, 120, ent->enemy, 120, MOD_BOMB_EXP);


	//show explosion
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

void Bomb_Exp_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	return;
}

void Throw_Bomb_Exp (edict_t *self)
{
	edict_t	*grenade;
	vec3_t	dir;
	int		f, r;

	//get random for right and forward
	f = (int)(random() * 4);
	r = (int)(random() * 4);
	f -= 2;
	r -= 2;

	VectorSet(dir, f, r, 3);
	VectorScale(dir, 70, dir);


	grenade = G_Spawn();

	VectorCopy (self->s.origin, grenade->s.origin);
	//grenade->s.origin[2] -= 10;
	VectorCopy (dir, grenade->velocity);

	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->owner = self->owner;
	grenade->touch = Bomb_Exp_Touch;
	grenade->nextthink = level.time + 0.25;
	grenade->think = Bomb_Exp_Explode;
	//grenade->s.modelindex = gi.modelindex ("models/objects/bombs/tris.md2");
	grenade->dmg = self->dmg;
	grenade->dmg_radius = self->dmg_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}


static void Bomb_Explode (edict_t *ent)
{
//	vec3_t		origin;
//	int			mod;

	//decrement counter
	ent->health --;

	//if counter is still going
	if (ent->health > 0)
	{
		//stop bomb in its tracks
		VectorClear (ent->velocity);
		ent->movetype = MOVETYPE_NONE;

		//change think to throw explosions
		Throw_Bomb_Exp(ent);
		ent->nextthink = level.time + 0.2;
		return;
	}

	//otherwise clear the entity

	//decrement shot and remove object
	remove_shot(ent);
}

static void Bomb_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
}

void fire_bomb (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	VectorSet (grenade->avelocity, 150, 150, 0);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	//VectorClear (grenade->mins);
	//VectorClear (grenade->maxs);
	VectorSet(grenade->mins, -6, -6, -6);
	VectorSet(grenade->maxs, 6, 6, 6);
	grenade->s.modelindex = gi.modelindex ("models/objects/bombs/tris.md2");
	grenade->owner = self;
	grenade->touch = Bomb_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Bomb_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	//counter for explosions
	grenade->health = 4;

	gi.linkentity (grenade);
}



void needle_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_NEEDLE);
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

	//decrement shot and remove object
	remove_shot(self);
}

void fire_needle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/needle/tris.md2");
	bolt->touch = needle_touch;
	bolt->s.effects |= EF_COLOR_SHELL;
	bolt->owner = self;
	bolt->nextthink = level.time + 3;
	bolt->think = remove_shot;
	bolt->dmg = damage;

	bolt->classname = "bolt";
	gi.linkentity (bolt);


	trace_t	tr;
	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}

#define MAGNET_WAIT		0.8

void magnet_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_MAGNET);
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_MAGNET_R);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	//decrement shot and remove object
	remove_shot(ent);
}

void Magnet_Think (edict_t *self)
{
	edict_t	*find, *ent;

	int		found;
	found = 0;

	//while we didnt tell it to exit
	//gi.cprintf (self->owner, PRINT_HIGH, "Searching\n");

	find = NULL;
	while ((find = findradius(find, self->s.origin, 400)) != NULL && found == 0)
	{
		if (find == NULL)
			continue;

		if (find == self)
			continue;

		if (find == self->owner)
			continue;

		if (!find->takedamage)
			continue;

		//if it is a takedamage and it has health and it's not the owner
		if (find->health > 0)
		{
			found = 1;
			ent = find;
			//gi.cprintf (self->owner, PRINT_HIGH, "We found something\n");
		}
	}

	//if we found something that is still alive
	if (found == 1)
	{

		//gi.cprintf (self->owner, PRINT_HIGH, "Aiming at that something\n");

		vec3_t	v;

		VectorSubtract (ent->s.origin, self->s.origin, v);

		vectoangles(v, self->s.angles);
		VectorNormalize(v);

		VectorCopy(v, self->movedir);
		VectorScale(v, 450, self->velocity);

		//play firing sound again when path changes
		gi.sound(self, CHAN_AUTO, gi.soundindex ("weapons/magnet.wav"), 1, ATTN_NORM, 0);
	}

	//think again
	self->nextthink = level.time + MAGNET_WAIT;

	self->health --; //decrement counter

	//if counter expires
	if (self->health == 0)
	{
		//gi.cprintf (self->owner, PRINT_HIGH, "It's going to die\n");
		//delete on next think
		self->think = remove_shot;
		self->nextthink = level.time + MAGNET_WAIT;
	}
}

void fire_magnet (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_BLASTER;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/magnet/tris.md2");
	rocket->owner = self;
	rocket->touch = magnet_touch;

	rocket->nextthink = level.time + MAGNET_WAIT;
	rocket->think = Magnet_Think;

	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	//rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	rocket->health = 4; //initialize counter for turns

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);

	trace_t	tr;
	tr = gi.trace (self->s.origin, NULL, NULL, rocket->s.origin, rocket, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (rocket->s.origin, -10, dir, rocket->s.origin);
		rocket->touch (rocket, tr.ent, NULL, NULL);
	}
}







/*
=================
fire_crashb
=================
*/
void crashb_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//if we hit a takedamage
	if (other->takedamage)
	{
		//if we did not already hit a takedamage
		if (ent->movetype == MOVETYPE_FLYMISSILE)
		{
			//do some damage
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_CRASHB);

			//drop crasb to floor
			VectorClear(ent->velocity);
			ent->movetype = MOVETYPE_BOUNCE;
		}

		//give some spin while falling
		VectorSet(ent->avelocity, 100, 100, 100);
	}
	else //otherwise we hit a wall
	{
		//stop the object
		VectorClear(ent->velocity);
		ent->movetype = MOVETYPE_NONE;

		//start the timer
		ent->nextthink = level.time + 1;
		ent->think = Bomb_Explode;
	}


	//play a sound
	gi.sound (ent, CHAN_AUTO, gi.soundindex("weapons/crash.wav") , 1, ATTN_NORM, 0);
}

void fire_crashb (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int dmg_radius)
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
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/crashb/tris.md2");
	rocket->owner = self;
	rocket->touch = crashb_touch;
	rocket->nextthink = level.time + 4;
	rocket->think = remove_shot;
	rocket->dmg = damage;
	rocket->dmg_radius = dmg_radius;
	rocket->classname = "rocket";

	//counter for explosions
	rocket->health = 4;

	VectorSet(rocket->avelocity, 0, 0, -800);

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}






void shrapnel_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//if we hit a takedamage do some damage
	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_SHRAPNEL);

		//kill the object
		G_FreeEdict(ent);
	}
}

void fire_shrapnel (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int type)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_BOUNCE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);

	if (type <= 2)
		rocket->s.modelindex = gi.modelindex ("models/objects/debris2/tris.md2");
	else if (type <= 4)
		rocket->s.modelindex = gi.modelindex ("models/objects/debris1/tris.md2");
	else
		rocket->s.modelindex = gi.modelindex ("models/objects/debris3/tris.md2");

	rocket->owner = self->owner;
	rocket->touch = shrapnel_touch;
	rocket->nextthink = level.time + 1.25;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->classname = "rocket";
	gi.linkentity (rocket);
}



void dust_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t mirrordir, v, f;
	int type, speed, damage;
	float r, u;

	//copy original velocity to mirrordir
	VectorCopy(ent->cutterangle, mirrordir);



	//other exists
	if (other)
	{
		//if we hit a takedamage
		if (other->takedamage)
		{
			// do some damage
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_DUST);

			//bounce back at opposite angle
			VectorNegate(mirrordir, mirrordir);
			speed = 1000;
		}
		//if we hit the floor
		else if (plane)
		{
			 if (plane->normal[2] != 0)
			{
				//we don't want shrapnel to hit the ground first and take a knockback
				//so reverse y direction to mirror angle
				mirrordir[2] *= -1;

				//give a normal speed
				speed = 1000;
				//gi.cprintf (ent->owner, PRINT_HIGH, "floor\n");
			}
			else //otherwise we hit a wall
			{
				//no need to do any reversals because the bounce will give us an appropriate angle
				//However the first bounce will reduce most of the velocity, so increase the speed
				speed = 1250;
				//gi.cprintf (ent->owner, PRINT_HIGH, "wall\n");
			}
		}
		else
		{
			//bounce back at opposite angle
			VectorNegate(mirrordir, mirrordir);
			speed = 1000;
		}
	}
	//if other does not exist
	else
	{
			//bounce back at opposite angle
			VectorNegate(mirrordir, mirrordir);
			speed = 1000;
	}

	//Throw 7 pieces of shrapnel
	for (int i = 0; i < 7; i++)
	{

		//randomize the model
		type = (int)(random() * 6);

		//randomize direction
		r = (int)(random() * 100);
		r -= 50;
		u = (int)(random() * 100);
		u -= 50;


		//determin new direction
		v[0] = mirrordir[0];
		v[1] = mirrordir[1] + r;
		v[2] = mirrordir[2] + u;

		VectorNormalize(v);

		//fire the shrapnel
		fire_shrapnel (ent, ent->s.origin, v, 8, speed, type);

	}


	//write a splash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	//decrement shot and remove object
	remove_shot(ent);
}

void fire_dust (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
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

	//we want to have a BBOX but it causes the game to crash
	//VectorSet (rocket->mins, -8, -8, -8);
	//VectorSet (rocket->maxs, 8, 8, 8);
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);

	rocket->s.modelindex = gi.modelindex ("models/objects/dust/tris.md2");
	rocket->owner = self;
	rocket->touch = dust_touch;
	rocket->nextthink = level.time + 4;
	rocket->think = remove_shot;
	rocket->dmg = damage;
	rocket->classname = "rocket";


	VectorCopy(rocket->velocity, rocket->cutterangle);
	gi.linkentity (rocket);


	trace_t	tr;
	tr = gi.trace (self->s.origin, NULL, NULL, rocket->s.origin, rocket, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (rocket->s.origin, -10, dir, rocket->s.origin);
		rocket->touch (rocket, tr.ent, NULL, NULL);
	}
}





//------------------------------
//Gemini Man - Gemini Laser
//------------------------------
void gemini_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_GEMINI);

		//decrement shot and remove object
		remove_shot(ent);
		return;
	}

	//otherwise we hit a wall so play bounce sound
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/geminihit.wav"), 1, ATTN_NORM, 0);


	ent->health--;
	if (ent->health == 0)
	{
		//decrement shot and remove object
		remove_shot(ent);
	}
}


void fire_gemini (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*laser;

	laser = G_Spawn();
	VectorCopy (start, laser->s.origin);
	VectorCopy (dir, laser->movedir);
	vectoangles (dir, laser->s.angles);
	VectorScale (dir, speed, laser->velocity);
	laser->movetype = MOVETYPE_FLYBOUNCE;
	laser->clipmask = MASK_SHOT;
	laser->solid = SOLID_BBOX;
	//VectorClear(laser->mins);
	//VectorClear(laser->maxs);

	//special thanks to LM_Jormungard
	//http://webadvisor.aupr.edu/noc/Othertutorials/qdevels/-%20Star%20Wars%20Blaster%20.html
	laser->s.renderfx |= RF_BEAM;
	laser->s.modelindex = 1;

	laser->s.frame = 3;

	// set the color
	laser->s.skinnum = 0xf3f3f1f1;

	VectorSet (laser->mins, -8, -8, -8);
	VectorSet (laser->maxs, 8, 8, 8);


	laser->owner = self;
	laser->touch = gemini_touch;
	laser->nextthink = level.time + 2.5;
	laser->think = remove_shot;
	laser->dmg = damage;
	laser->classname = "laser";

	//start counter
	laser->health = 4;

	//VectorCopy(laser->velocity, laser->cutterangle);
	gi.linkentity (laser);

	trace_t	tr;
	tr = gi.trace (self->s.origin, NULL, NULL, laser->s.origin, laser, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (laser->s.origin, -10, dir, laser->s.origin);
		laser->touch (laser, tr.ent, NULL, NULL);
	}
}




void skull_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->owner == ent->owner)
		return;

	if (other->takedamage)
	{
		//do some damage
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_SKULL);

		//decrement shot and remove object
		remove_shot(ent);
	}
}

#define SKULL_STEP	5

void skull_think (edict_t *ent)
{
	//decrement life counter
	ent->radius_dmg--;

	//kill if at 0
	if (ent->radius_dmg == 0 ||
		ent->owner->health <= 0 ||
		ent->owner->client->pers.enhance == ENH_RTANK)
	{
		//decrement shot and remove object
		remove_shot(ent);
		return;
	}


	//increment spin counter
	ent->count++;

	//reset counter to 1 to restart loop
	if (ent->count > 40)
		ent->count = 1;

	//first 10 moves
	if (ent->count <= 10)
	{
		//adjust displacement northwest
		ent->cutterangle[0]-= SKULL_STEP;
		ent->cutterangle[1]+= SKULL_STEP;
	}
	else if (ent->count <= 20)
	{
		//adjust displacement southwest
		ent->cutterangle[0]-= SKULL_STEP;
		ent->cutterangle[1]-= SKULL_STEP;
	}
	else if (ent->count <= 30)
	{
		//adjust displacement southeast
		ent->cutterangle[0]+= SKULL_STEP;
		ent->cutterangle[1]-= SKULL_STEP;
	}
	else
	{
		//adjust displacement northeast
		ent->cutterangle[0]+= SKULL_STEP;
		ent->cutterangle[1]+= SKULL_STEP;
	}

	//recalculate position
	VectorAdd(ent->owner->s.origin, ent->cutterangle, ent->s.origin);

	//make sure we face the same dir as player
	VectorCopy(ent->owner->s.angles, ent->s.angles);


	//print results to be sure of what happened
	//int x, y;
	//x = (int) ent->cutterangle[0];
	//y = (int) ent->cutterangle[1];
	//gi.cprintf (ent->owner, PRINT_HIGH, "x: %i y: %i  count: %i \n", x, y, ent->count);


	ent->nextthink = level.time + 0.01;
}

void skull_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

	//decrement shot and remove object
	remove_shot(self);
}



void fire_skulls (edict_t *self, int damage, int height)
{
	edict_t	*rocket;

	rocket = G_Spawn();

	if (height == 0)
	{
		//set beginning displacement
		VectorSet(rocket->cutterangle, (SKULL_STEP * 10), 0, -5);

		//begin counter for spin
		rocket->count = 0;
	}
	else if (height == 1)
	{
		//set beginning displacement
		VectorSet(rocket->cutterangle, 0, (SKULL_STEP * 10), 7);

		//begin counter for spin
		//already displaced so set counter higher
		rocket->count = 10;
	}
	else if (height == 2)
	{
		//set beginning displacement
		VectorSet(rocket->cutterangle, (SKULL_STEP * -10), 0, 19);

		//begin counter for spin
		//already displaced so set counter higher
		rocket->count = 20;
	}
	else
	{
		//set beginning displacement
		VectorSet(rocket->cutterangle, 0, (SKULL_STEP * -10), 31);

		//begin counter for spin
		//already displaced so set counter higher
		rocket->count = 30;
	}

	//displace skull from player
	VectorAdd (self->s.origin, rocket->cutterangle, rocket->s.origin);

	//make sure there's no movement
	VectorClear (rocket->movedir);
	VectorClear(rocket->velocity);

	//
	VectorCopy (self->s.angles, rocket->s.angles);

	//initialize life counter.. 2 seconds
	rocket->radius_dmg = 200;


	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;

	VectorSet (rocket->mins, -16, -16, -16);
	VectorSet (rocket->maxs, 16, 16, 16);
	//VectorClear (rocket->mins);
	//VectorClear (rocket->maxs);

	rocket->s.modelindex = gi.modelindex ("models/objects/gibs/skull/tris.md2");
	rocket->owner = self;

	rocket->touch = skull_touch;

	rocket->nextthink = level.time + 0.01;
	rocket->think = skull_think;
	rocket->dmg = damage;
	rocket->classname = "rocket";

	rocket->health = 100;
	rocket->takedamage = true;
	rocket->die = skull_die;

	gi.linkentity (rocket);
}


//------------------------
//Zero's Sabre
//------------------------
void sabre_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	//if it can be damaged, hit it
	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_SABRE);
	}

	//throw some sparks
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLASTER);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void sabre_think (edict_t *self)
{
	//decrement timer
	self->health--;

	//if timer is up
	if (self->health == 0)
	{
		//kill shot and decrement shot count
		remove_shot(self);
		return;
	}

	vec3_t v, f, r;

	//make sure we have no velocity
	VectorClear(self->velocity);

	//adjust angle based on movement so far
	v[0] = self->owner->client->v_angle[0] += self->cutterangle[0];
	v[1] = self->owner->client->v_angle[1] += self->cutterangle[1];
	v[2] = self->owner->client->v_angle[2];

	//find the forward and angles
	AngleVectors (v, f, r, NULL);

	//make laser face the new direction
	vectoangles(f, self->s.angles);

	//normalize forward and scale it to a useful position
	VectorNormalize(f);
	VectorScale(f, 50, f);

	//add forward to owner's origin and put into sabres origin
	VectorAdd(f, self->owner->s.origin, self->s.origin);
	//readjust for height
	self->s.origin[2] += (self->owner->viewheight - 8);

	//normalize right and scale it to a useful distance
	VectorNormalize(r);
	VectorScale(r, 3, r);
	//add right to sabre origin
	VectorAdd(r, self->s.origin, self->s.origin);

	//make old_origin into player origin for drawing the laser
	//VectorCopy(self->owner->s.origin, self->s.old_origin);
	//adjust old_origin for height
	//self->s.old_origin[2] += (self->owner->viewheight - 8);
	//fix right for old origin
	//VectorAdd(r, self->s.old_origin, self->s.old_origin);


	//begin movement down and left
	self->cutterangle[0] += 13;
	self->cutterangle[1] += 13;

	//think again soon
	self->nextthink = level.time + 0.01;
}

void fire_sabre (edict_t *self, int damage, int speed)
{
	edict_t	*laser;

	laser = G_Spawn();
	VectorCopy (self->s.origin, laser->s.origin);

	VectorClear (laser->movedir);
	VectorClear (laser->s.angles);
	VectorClear (laser->velocity);
	laser->movetype = MOVETYPE_FLYBOUNCE;
	laser->clipmask = MASK_SHOT;
	laser->solid = SOLID_BBOX;


	//special thanks to LM_Jormungard
	//http://webadvisor.aupr.edu/noc/Othertutorials/qdevels/-%20Star%20Wars%20Blaster%20.html
	//laser->s.renderfx |= RF_BEAM;
	laser->s.modelindex = 0;

	//laser->s.frame = 3;

	// set the color to red
	//laser->s.skinnum = 0xf2f2f0f0;

	VectorSet (laser->mins, -24, -24, -24);
	VectorSet (laser->maxs, 24, 24, 24);


	laser->owner = self;
	laser->touch = sabre_touch;
	laser->nextthink = level.time + 0.01;
	laser->think = sabre_think;
	laser->dmg = damage;
	laser->classname = "laser";

	//start counter
	laser->health = 5;
	VectorSet(laser->cutterangle, -20, -20, 0);

	//VectorCopy(laser->velocity, laser->cutterangle);
	gi.linkentity (laser);
}

//peewee end
