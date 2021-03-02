#include "g_local.h"



/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
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

if (wfdebug) gi.dprintf("Fire Lead...\n");

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
//ERASER START
		// long traceline's eat CPU
		if (self->bot_client)
		{
			VectorMA (start, 1000, forward, end);
			r = (1/8) * r;
			r = (1/8) * u;
		}
		else
//ERASER END
			VectorMA (start, 8192, forward, end);

		//Don't add random spread if it's the sniper riffle
		if (mod != MOD_SNIPERRIFLE)
		{
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);
		}

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
	else
	{
if (wfdebug) gi.dprintf(" --didn't hit anything!\n");
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
if (wfdebug) gi.dprintf(" --hit something: %s\n", tr.ent->classname);
//			if (tr.ent->takedamage)
			if ((tr.ent->takedamage) && (tr.ent->wf_team != self->wf_team))
			{
				if (self->owner)
					T_Damage (tr.ent, self, self->owner, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
				else
					T_Damage (tr.ent, self, self,        aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
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
		else
		{
if (wfdebug) gi.dprintf(" --nothing in the middle\n");
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
fire_plasmabeam

Fires PlasmaBeam. Used in new hyperblaster
by Fireball
=================
*/
void fire_plasmabeam (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end, offset;
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
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_HYPERBLASTER);
		}

		VectorCopy (tr.endpos, from);
	}
   VectorCopy (start, offset);

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_HEATBEAM);

	gi.WriteShort (self - g_edicts);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (start, MULTICAST_PHS);
/*
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}
*/
	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
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
//WF: don't stop if it hits.  remove the return statement for normal use
//		return;
//WF

		//Original code
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

//Original blaster shooting
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

//WF - New fire blaster for lasers
void fire_blaster_wf (edict_t *self, vec3_t start, vec3_t dir, int damage,  int speed, int effect, qboolean hyper)
     {
        edict_t    *bolt;
        trace_t     tr;
        vec3_t      from;
        vec3_t      end;

        VectorNormalize (dir);

        if(effect & EF_BLASTER)// if blaster, spawn blaster bolt
        {
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
           bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
           bolt->s.sound = gi.soundindex("misc/lasfly.wav");
           bolt->owner = self;
           bolt->touch = blaster_touch;
           bolt->nextthink = level.time + 2;
           bolt->think = G_FreeEdict;
           bolt->dmg = damage;
           bolt->classname = "bolt";

           gi.linkentity (bolt);

           if (self->client)
              check_dodge (self, bolt->s.origin, dir, speed);

           tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt,
     MASK_SHOT);
           if (tr.fraction < 1.0)
           {
              VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
              bolt->touch (bolt, tr.ent, NULL, NULL);
           }
        }
        else // laser hyperblaster
        {
           // set origin of laser beam at gun barrel.
           // note that the barrel is rotating, so the beams will
           // originate from different places each time.
           VectorMA (start, 8192, dir, end);
           VectorCopy (start, from);
           // trace for end point of laser beam.
           // the laser aim is perfect.
           // no random aim like the machinegun
           tr = gi.trace (from, NULL, NULL, end, self, MASK_SHOT);
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
     damage, 0, 0, MOD_HYPERBLASTER);
           else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
           {  // hit a brush, send clients
              // a light flash and sparks temp entity.
              gi.WriteByte (svc_temp_entity);
              gi.WriteByte (TE_BLASTER);
              gi.WritePosition (tr.endpos);
              gi.WriteDir (tr.plane.normal);
              gi.multicast (self->s.origin, MULTICAST_PVS);
           }
        }
     }

/*
=================
fire_grenade
=================
*/
//WF
//static void Grenade_Explode (edict_t *ent)
//WF
void Grenade_Explode (edict_t *ent)
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

//WF safe version of the explosion
void GenericGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	if (ent->mod)
		mod = ent->mod;
	else if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	//removed enemy code

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

void GenericGrenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = GenericGrenade_Explode;
}

void Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Grenade_Explode;
}

void Rocket_Explode (edict_t *ent)
{
	int		i;	vec3_t	org;	float		spd;
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, ent->mod);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
// make some glowing shrapnel
	spd = 15.0 * ent->dmg / 200;
	for (i = 0; i < 3; i++)
	{
		org[0] = ent->s.origin[0] + crandom() * ent->size[0];
		org[1] = ent->s.origin[1] + crandom() * ent->size[1];
		org[2] = ent->s.origin[2] + crandom() * ent->size[2];
		ThrowShrapnel4 (ent, "models/objects/debris2/tris.md2", spd, org);
	}
	make_debris (ent);
	if ((ent->homing_lock) && (ent->mod != MOD_STINGER))
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius,MOD_HOMINGROCKET);
	else
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius, ent->mod);
	//T_ShockItems(ent);
	//T_ShockWave(ent, 255, 1024);
	BecomeNewExplosion (ent);
}

void Rocket_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Rocket_Explode;
}

void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		int	i;

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
//ERASER START
		// look for bot's to alarm
		for (i=0; i<num_players; i++)
		{
			if ((players[i]->bot_client) && (players[i]->health > 0) && (entdist(players[i], ent) < 256))
			{
				players[i]->avoid_ent = ent;
			}
		}
//ERASER END
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
//	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->s.modelindex = gi.modelindex (GRLAUNCHER_MODEL);
	grenade->s.skinnum = GRLAUNCHER_SKIN;
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
//	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->s.modelindex = gi.modelindex (GRNORMAL_MODEL);
	grenade->s.skinnum = GRNORMAL_SKIN;
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


// WF & CCH: New think function for homing missiles
void homing_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t  targetdir, blipdir;
	vec_t   speed;

	while ((blip = findradius(blip, ent->s.origin, 750)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
		{
			//The only non-client to track is turret grenades and sentry guns
			if ( (strcmp(blip->classname, "turret") != 0) &&
				 (strcmp(blip->classname, "SentryGun") != 0) &&
				 (strcmp(blip->classname, "laser_defense_gr") != 0))
			{
				continue;
			}
		}
		if (blip == ent->owner)
			continue;
		//dont aim at same team unless friendly fire is on
  	    if ((blip->wf_team == ent->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
		//if (blip->wf_team == ent->wf_team)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}

	if (target != NULL)
	{
		 // target acquired, nudge our direction toward it
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.135, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);

		//is this the first time we locked in? sound warning for the target
		if (ent->homing_lock == 0)
		{
			gi.sound (target, CHAN_AUTO, gi.soundindex ("homelock.wav"), 1, ATTN_NORM, 0);
//			gi.sound (target, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
			ent->homing_lock = 1;
		}
	}

	ent->nextthink = level.time + .1;
}
//WF

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
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, ent->mod);
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

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, ent->mod);
//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

//ERASER START
void RocketInformDanger(edict_t *self)
{
	int	i;
	edict_t	*bot;
	vec3_t	org2, vec;
	float	dist;

	if (self->timestamp < (level.time - 5))
	{
		G_FreeEdict(self);
		return;
	}

	VectorMA(self->s.origin, 0.25, self->velocity, org2);

	// scan for bot's to inform
	for (i=0; i<num_players; i++)
	{
		bot = players[i];

		if (bot->bot_client && (bot->bot_stats->combat < 4))
			continue;

		if ((fabs(bot->s.origin[0] - self->s.origin[0]) > 300) ||
			(fabs(bot->s.origin[1] - self->s.origin[1]) > 300))
			continue;

		// make sure rocket is heading somewhat towards this bot
		VectorSubtract(org2, bot->s.origin, vec);
		if (((dist=entdist(bot, self)) - VectorLength(vec)) > 75)
		{	// yup, inform bot of danger
			bot->avoid_ent = self;
		}
	}

	self->nextthink = level.time + 0.3;
}
//ERASER END

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, int mod)
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

//WF & CCH - Add homing lock status
	rocket->homing_lock = 0;

	rocket->mod = mod;

	//Set the team of the rocket
	rocket->wf_team = self->wf_team;

	//Turn off homing state if the WF_NO_HOMING flag is set
	//This is set here so the feature can be turned off mid-game
	if (self->client && ((int)wfflags->value & WF_NO_HOMING))
		self->client->pers.homing_state = 0;

//Eraser START: Modified to scan for Bots to inform of danger
	rocket->timestamp = level.time;
	rocket->nextthink = level.time + 0.1;
	rocket->think = RocketInformDanger;
//ERASER END
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;

	// See if this is a player and if they have homing on
	if (self->client && self->client->pers.homing_state)
	{
		// If they have 5 cells, start homing, otherwise normal rocket think
		if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= 5)
		{
			self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 5;
			rocket->nextthink = level.time + .1;
			rocket->think = homing_think;

			//reduce damage of homing rockets to 1/2 of normal rocket
			rocket->dmg = damage / 2;

			if (mod == MOD_ROCKET) mod = MOD_HOMINGROCKET;
		}
		else {
			safe_cprintf(self, PRINT_HIGH, "No cells for homing missile.\n");
			rocket->nextthink = level.time + 8000/speed;
			rocket->think = G_FreeEdict;
		}
	}
	else {
		rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
        }
//WF

	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

#define STANDING_HEAD_POS	20	//above center of body
#define STANDING_LEG_POS	12	//below center of body
#define CROUCHED_HEAD_POS	1	//above center of body //Acrid 5/99 was 8
#define CROUCHED_LEG_POS	22	//below center of body //Acrid fix this too?

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
	qboolean	ducked;
	int			leg_pos;
	int			head_pos;
	char		*name;

	int		bodypos = 0;	//Where on the body was the hit?

if (wfdebug) gi.dprintf("Rail start damage = %d\n", damage);

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

  			//5/99 Acrid ,to tell your the owner, so you dont shoot yourself on top of a dead body
			if (tr.ent == self)
				return;

			//Location damage!  Was it a head or leg shot?
			if (tr.ent->client && mod == MOD_SNIPERRIFLE)
			{
				if (tr.ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					head_pos = CROUCHED_HEAD_POS;
					leg_pos = CROUCHED_LEG_POS;
					ducked = true;
				}
				else
				{
					head_pos = STANDING_HEAD_POS;
					leg_pos = STANDING_LEG_POS;
					ducked = false;
				}

				if (tr.endpos[2] < (tr.ent->s.origin[2] - leg_pos))
				{
					bodypos = 1;  // leg shot
					mod = MOD_SNIPERRIFLE_LEG;
					damage = damage * .333;  // 1/3 damage for leg shot
					//Dont slow them down if it's a team mate
					if (tr.ent->wf_team != self->wf_team)
					{
						tr.ent->lame = 1;	//slow them down!
						if (tr.ent->client)
						{
							if (self->client) name = &self->client->pers.netname[0];
							else name = "(unknown)";
							safe_cprintf(tr.ent, PRINT_HIGH, "You were hit in the leg by %s!\n", name);
						}
						if (self->client)
						{
							if (tr.ent->client) name = &tr.ent->client->pers.netname[0];
							else name = "(unknown)";
                            safe_cprintf(self, PRINT_HIGH, "You shot %s in the leg!\n",name);
						}
					}
				}
				else if (tr.endpos[2] > ((tr.ent->s.origin[2] + head_pos)))
				{
					bodypos = 2;
					mod = MOD_SNIPERRIFLE_HEAD;
					damage = damage * 3;  // Increase damage for head shot

					//Squirt some blood!
//gi.dprintf("HEADSHOT\n");
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (60);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (SPLASH_BLOOD);
					gi.multicast (tr.endpos, MULTICAST_PVS);


					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_BLOOD);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}
				else
				{
					bodypos = 0;
				}
			}
			else if (!tr.ent->client)
			{
				//For non-clients, do less damage
				damage = damage * .7;
			}

			if (mod == MOD_WF_TURRET)
				T_Damage (tr.ent, self, self->owner, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);
			else
				T_Damage (tr.ent, self, self,        aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);
if (wfdebug)
{
	gi.dprintf("Rail adjusted damage = %d.", damage);
	if (tr.ent->client )
		gi.dprintf(": %s\n", tr.ent->client->pers.netname);
	else
		gi.dprintf("\n");

}
		}

		VectorCopy (tr.endpos, from);
	}


	// send gun puff / flash

	//Dont do railgun train if sniper or sentry gun
//	if (((int)wfflags->value & WF_NO_RAILGUN_EFFECT) || (mod == MOD_SNIPERRIFLE) || mod == MOD_SENTRY)
	if (mod == MOD_SNIPERRIFLE || mod == MOD_SENTRY  || mod == MOD_SNIPERRIFLE_LEG || mod == MOD_SNIPERRIFLE_HEAD)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.multicast (tr.endpos, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
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
//WF - reduce the damage of bfg
//                dmg = 5;
                dmg = 2;
//WF
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
//        bfg->s.modelindex = gi.modelindex ("sprites/fire.sp2");
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