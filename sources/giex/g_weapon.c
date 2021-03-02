#include "g_local.h"

void notifySplashResult(edict_t *inflictor, int damage) {
	//Set a position on monsters to let them know they hit their intended target with splash damage.
	if ((damage > 0) && (inflictor->owner->classid == CI_M_TANK || inflictor->owner->classid == CI_M_TANKCOMM)) {
		VectorCopy(inflictor->s.origin, inflictor->owner->pos1);
		inflictor->owner->pos2[0] = 0;
		inflictor->owner->pos2[1] = 0;
		inflictor->owner->pos2[2] = 0;
	}
}

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
void check_dodge_area(edict_t *origin, vec3_t center, int radius, float jump) {
	edict_t *scan = NULL;
	vec3_t dir;
	if (jump <= 0)
		return;

	while ((scan = findradius(scan, center, radius)) != NULL) {
		if (!scan->inuse)
			continue;
		if (!(scan->svflags & SVF_MONSTER))
			continue;
		if (scan->health < 1)
			continue;
		if (scan->monsterinfo.jumptime > level.time)
			continue;
		if (!infront(scan, origin))
			continue;
		if (random() > jump)
			continue;
		if (abs(scan->velocity[2]) > 10)
			continue;

		VectorSubtract(scan->s.origin, center, dir);
		VectorNormalize(dir);
		if (VectorLength(dir) == 0) {
			scan->velocity[0] = crandom() * 500;
			scan->velocity[1] = crandom() * 500;
		} else {
			VectorScale(dir, 600, scan->velocity);
		}
		scan->velocity[2] = (int) ceil(160000.0 / (scan->mass + 500));
		scan->monsterinfo.jumptime = level.time + 2.0;
	}
}
void check_dodge_rocket(edict_t *self, vec3_t start, vec3_t dir, int speed, int range, int radius, float jump, float dodge) {
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float eta;

	if (range == 0) {
		VectorMA (start, 8192, dir, end);
	} else {
		VectorMA (start, range, dir, end);
	}
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.fraction < 1.0) {
		check_dodge_area(self, tr.endpos, radius, jump);
	}
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && infront(tr.ent, self)) {
		if ((dodge > 0) && (tr.ent->monsterinfo.dodge) && (tr.ent->s.origin[2] > (self->s.origin[2] + 16)) && (random() < dodge)) {
			VectorSubtract (tr.endpos, start, v);
			eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
			tr.ent->monsterinfo.dodge (tr.ent, self, eta);
		}
	}
}
void check_dodge(edict_t *self, vec3_t start, vec3_t dir, int speed, int range, float jump, float dodge) {
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

//	if (random() > 0.25)
//		return;
	if (range == 0) {
		VectorMA (start, 8192, dir, end);
	} else {
		VectorMA (start, range, dir, end);
	}
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && infront(tr.ent, self)) {
		if ((jump > 0) && (tr.ent->monsterinfo.jumptime < level.time) && (random() < jump) && (abs(tr.ent->velocity[2]) < 10)) {
			tr.ent->velocity[0] = crandom() * 500;
			tr.ent->velocity[1] = crandom() * 500;
			tr.ent->velocity[2] = (int) ceil(160000.0 / (tr.ent->mass + 500));
			tr.ent->monsterinfo.jumptime = level.time + 2.0;
		}
		if ((dodge > 0) && (tr.ent->monsterinfo.dodge) && (tr.ent->s.origin[2] > (self->s.origin[2] + 16)) && (random() < dodge)) {
			VectorSubtract (tr.endpos, start, v);
			eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
			tr.ent->monsterinfo.dodge (tr.ent, self, eta);
		}
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

	if (!self->enemy)
		return false;
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
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
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
	if (self->client)
		check_dodge (self, start, aimdir, 2500, 0, 0.2, 0.0);
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
	if (self->client)
		check_dodge (self, start, aimdir, 2500, 0, 0.1, 0.0);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {
	int		mod;

	if (!self->owner) {
		G_FreeEdict (self);
		return;
	}
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) {
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_ENERGY, mod);
	} else {
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

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper) {
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
	bolt->classid = CI_BOLT;
	bolt->classname = "blasterbolt";
	if (hyper)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed, 0, 0.4, 0.5);

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
void Grenade_Explode (edict_t *ent) {
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
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(NULL, ent, ent->owner, ent->dmg, ent->dmg * 0.5, ent->enemy, ent->dmg_radius, true, mod);

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
			if (ent->owner->client) {
				iteminfo_t *info = getWornItemInfo(ent->owner, 0);
				float newtime = level.time + info->arg9 + info->arg10 * ent->owner->client->pers.skill[12];
				if ((ent->count >= info->arg8) && (newtime < ent->nextthink) && (newtime > level.time))
					ent->nextthink = newtime;
				else
					ent->count++;
			}
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float gravity, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, (200 * gravity) + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	if (self->client && (self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 4)) {
		self->client->silencer_shots -= 1;
	} else {
		grenade->s.effects |= EF_GRENADE;
	}
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->gravity = gravity;
	grenade->classid = CI_GRENADE;
	grenade->classname = "grenade";

	if (self->client)
		check_dodge_rocket(self, grenade->s.origin, dir, speed, 512, damage_radius, 0.45, 0.0);

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
	if (self->client && (self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 4)) {
		self->client->silencer_shots -= 2;
	} else {
		grenade->s.effects |= EF_GRENADE;
		grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
	}
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classid = CI_HGRENADE;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
		if (self->client)
			check_dodge_rocket(self, grenade->s.origin, dir, speed, 512, damage_radius, 0.45, 0.0);
	}
}


/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
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

	if (ent->owner->svflags & SVF_MONSTER) {
		notifySplashResult(ent, T_RadiusDamage(ent->owner->enemy, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, other, ent->dmg_radius, true, MOD_R_SPLASH));
	} else {
		T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, other, ent->dmg_radius, true, MOD_R_SPLASH);
	}

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

edict_t *fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage) {
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if (self->client) {
		if ((self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 4)) {
			self->client->silencer_shots -= 1;
		} else {
			rocket->s.effects |= EF_ROCKET;
		}
	} else if (self->svflags & SVF_MONSTER) {
		if ((self->classid == CI_M_TANK) || (self->classid == CI_M_TANKCOMM)) { //Rockets from tanks doesn't leave trails.
			rocket->s.effects |= EF_ROCKET;
		}
	} else {
		rocket->s.effects |= EF_ROCKET;
	}
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
	rocket->classid = CI_ROCKET;
	rocket->classname = "rocket";

	if (self->client)
		check_dodge_rocket(self, rocket->s.origin, dir, speed, 0, damage_radius, 0.4, 0.0);

	gi.linkentity (rocket);
	return rocket;
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
	int i = 20;
	qboolean	water;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore && (i > 0))
	{
		i--;
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
	if (self->client && (self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 9)) {
		self->client->silencer_shots -= 1;
	} else {
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
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
//	if (self->client)
//		check_dodge (self, start, aimdir, 10000, 0.05, 0.0);
}


/*
=================
fire_bfg
=================
*/
/*
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
*/
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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0/*self->dmg*/, 0, MOD_BFG_BLAST);
	if (self->owner->svflags & SVF_MONSTER) {
		notifySplashResult(self, T_RadiusDamage(self->owner->enemy, self, self->owner, self->radius_dmg, self->radius_dmg * 0.5, other, self->dmg_radius, true, MOD_BFG_BLAST));
	} else {
		T_RadiusDamage(NULL, self, self->owner, self->radius_dmg, self->radius_dmg * 0.5, other, self->dmg_radius, true, MOD_BFG_BLAST);
	}

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

/*
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
*/

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
	bfg->dmg = damage;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classid = CI_BFGBLAST;
	bfg->classname = "bfgblast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

/*	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;*/

	if (self->client)
		check_dodge_rocket(self, bfg->s.origin, dir, speed, 0, damage_radius, 0.7, 0.0);

	gi.linkentity (bfg);
}



static qboolean fire_armtrail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int range, qboolean hitflash, int mod) {
	qboolean result = false;
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	r = crandom() * ((float) range / 8192.0) * hspread;
	u = crandom() * ((float) range / 8192.0) * vspread;
	VectorMA (start, range, forward, end);
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);

	if (tr.ent && tr.ent->takedamage) {
		if (tr.ent->groundentity) {
			kick *= 4;
		}
		T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, 0 /*kick*/, DAMAGE_25_PIERCE, mod);

		if (hitflash) {
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (tr.ent-g_edicts);
			gi.WriteByte (MZ_NUKE1);
			gi.multicast (tr.ent->s.origin, MULTICAST_PVS);
		}

		result = true;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BUBBLETRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (start, MULTICAST_PVS);

	return result;
}

void fire_armblast (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int range, int mod) {
	int		i;
	qboolean hit = false;

	for (i = 0; i < count; i++) {
		if (fire_armtrail (self, start, aimdir, damage, (hit ? 0 : kick), TE_SHOTGUN, hspread, vspread, range, !hit, mod)) {
			hit = true;
		}
	}
	if (self->client)
		check_dodge (self, start, aimdir, 2500, 0, 0.1, 0.0);
}

/*
===============
fire_incendiary
===============
*/
void thinkIncendiary (edict_t *ent) {
	float mult = 1.0;
	ent->nextthink = level.time + FRAMETIME;

	if (ent->count >= 5)
		mult = 5.0 / (float) (10 - ent->count);
	T_RadiusDamage(NULL, ent, ent->owner, ent->dmg * mult, ent->dmg * mult * 0.5, NULL, ent->dmg_radius, true, MOD_INCENDIARY);

	ent->count--;
	if (ent->count < 0) {
		ent->think = G_FreeEdict;
	}
}

void spawnIncendiary(edict_t *owner, vec3_t start, int dmg, int radius, int speed) {
	edict_t *ent;
	vec3_t aimdir;
	double dir = random() * 2 * M_PI;

	aimdir[0] = cos(dir);
	aimdir[1] = sin(dir);
	aimdir[2] = random() * 0.6 + 0.4;

	ent = G_Spawn();
	ent->classid = CI_INCENDIARY;
	ent->classname = "incendiary";
	ent->owner = owner;
	VectorCopy (start, ent->s.origin);
	VectorScale (aimdir, speed, ent->velocity);
	VectorSet (ent->avelocity, 300, 300, 300);
	ent->movetype = MOVETYPE_TOSS;
	//ent->clipmask = MASK_NONE;
	ent->solid = SOLID_NOT;
	ent->s.effects = EF_TELEPORTER;
	ent->count = 25;
	ent->dmg = dmg;
	ent->dmg_radius = radius;
	ent->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	ent->think = thinkIncendiary;
	ent->nextthink = level.time + FRAMETIME;
	ent->gravity = 0.5;
//	gi.setmodel(ent, "models/objects/dmspot/tris.md2");
	
	gi.linkentity(ent);
}

static void Incendiary_Explode (edict_t *ent) {
	vec3_t		origin;
	int i;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->dmg > 0) {
		if (ent->enemy) {
			float	points;
			vec3_t	v;
			vec3_t	dir;

			VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
			VectorMA (ent->enemy->s.origin, 0.5, v, v);
			VectorSubtract (ent->s.origin, v, v);
			points = ent->dmg - 0.5 * VectorLength (v);
			VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
			T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_RADIUS, MOD_INCENDIARY_GRENPOP);
		}

		T_RadiusDamage(NULL, ent, ent->owner, ent->dmg, ent->dmg * 0.5, ent->enemy, ent->dmg_radius, true, MOD_INCENDIARY_GREN);
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

	for(i = 0; i < 10; i++) {
		spawnIncendiary(ent->owner, ent->s.origin, ent->radius_dmg, 150, 270);
	}

	G_FreeEdict (ent);
}

static void Incendiary_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
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
			if (ent->owner->client) {
				iteminfo_t *info = getWornItemInfo(ent->owner, 0);
				float newtime = level.time + info->arg9 + info->arg10 * ent->owner->client->pers.skill[12];
				if ((ent->count >= info->arg8) && (newtime < ent->nextthink) && (newtime > level.time))
					ent->nextthink = newtime;
				else
					ent->count++;
			}
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Incendiary_Explode (ent);
}

void fire_incendiary(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int incendiary_damage, int speed, float gravity, float timer, float damage_radius) {
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, (200 * gravity) + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	if (self->client && (self->client->silencer_shots > 0) && (self->client->pers.skill[58] > 4)) {
		self->client->silencer_shots -= 1;
	} else {
		grenade->s.effects |= EF_GRENADE;
	}
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Incendiary_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Incendiary_Explode;
	grenade->dmg = damage;
	grenade->radius_dmg = incendiary_damage;
	grenade->dmg_radius = damage_radius;
	grenade->gravity = gravity;
	grenade->classid = CI_INCENDIARYGRENADE;
	grenade->classname = "incendiarygrenade";

	if (self->client)
		check_dodge_rocket(self, grenade->s.origin, dir, speed, 512, damage_radius, 0.45, 0.0);

	gi.linkentity (grenade);
}
