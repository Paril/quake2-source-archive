#include "g_local.h"
#include "g_brrr.h"

void SV_CheckVelocity (edict_t *ent);
trace_t SV_PushEntity (edict_t *ent, vec3_t push);
int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce);

/* *** NUKE *** */
void nukeimpacteffect(edict_t *ent)
{
   	gi.WriteByte (svc_temp_entity);
   	gi.WriteByte (TE_EXPLOSION1_BIG);
   	gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PHS);
    ent->nextthink=level.time+0.1;
}
/* *** */

/* *** GUIDED ROCKETS *** */
void guided_sparks(edict_t *ent)
{
  vec3_t  forward, right;
  vec3_t  pack_pos, jet_vector;

  AngleVectors(ent->s.angles, forward, right, NULL);
  VectorScale (forward, -7, pack_pos);
  VectorAdd (pack_pos, ent->s.origin, pack_pos);
  pack_pos[2] += (ent->viewheight);
  VectorScale (forward, -50, jet_vector);

  gi.WriteByte (svc_temp_entity);
  gi.WriteByte (TE_SPARKS);
  gi.WritePosition (pack_pos);
  gi.WriteDir (jet_vector);
  gi.multicast (pack_pos, MULTICAST_PVS);
}

void guided_die_think(edict_t *ent)
{
    vec3_t      forward;

    if ((int)ent->s.angles[PITCH]!=90)
    ent->s.angles[PITCH]++;
    ent->s.angles[ROLL]+=10;
    ent->s.angles[YAW]+=10;

    AngleVectors(ent->s.angles, forward, NULL, NULL);
	VectorScale(forward, 250, ent->velocity);

    ent->nextthink=level.time+0.1;

    guided_sparks(ent);
}

void guided_die(edict_t *ent)
{
	if (ent->owner && ent->owner->client)
    {
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
        ent->owner->client->rocketview=false;
    	ent->owner->client->missile=NULL;

        if (ent->owner->client->pers.weapon)
            ent->owner->client->ps.gunindex = gi.modelindex(ent->owner->client->pers.weapon->view_model);
    }

    ent->takedamage=DAMAGE_NO;
    ent->die=NULL;

    ent->think=guided_die_think;
    ent->nextthink=level.time+0.1;
}

void nuke_die(edict_t *ent)
{
    ent->classname="rocket";
	ent->dmg=110;
	ent->radius_dmg=120;
	ent->dmg_radius=120;

    ent->takedamage=DAMAGE_NO;
    ent->die=NULL;

    ent->think=guided_die_think;
    ent->nextthink=level.time+0.1;
}
/* *** */

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

	if (self->spawnflags & 1)
		mod = MOD_HYPERBLASTER;
	else
		mod = MOD_BLASTER;

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		/* *** RICHOCHET *** */
		if (mod==MOD_HYPERBLASTER || !((unsigned long)ann_allow->value&(unsigned long)ANN_ALLOW_RICOCHETBLASTER))
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
		else
		{
			/* Return early so blaster shots can ricochet */
			return;
		}
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;
	vec3_t	from;
	vec3_t	end;

	VectorNormalize (dir);

	if (!hyper || (hyper && self->client && !self->client->resp.hlaser))
	{
		bolt = G_Spawn();
		VectorCopy (start, bolt->s.origin);
		VectorCopy (start, bolt->s.old_origin);
		vectoangles (dir, bolt->s.angles);
		VectorScale (dir, speed, bolt->velocity);
		if (hyper)
		{
			bolt->movetype = MOVETYPE_FLYMISSILE;
		}
		/* *** RICOCHET *** */
		else
		{
			bolt->movetype = MOVETYPE_FLYRICOCHET;
		}
		/* *** */
		bolt->clipmask = MASK_SHOT;
		bolt->solid = SOLID_BBOX;
		bolt->s.effects |= effect;
		VectorClear (bolt->mins);
		VectorClear (bolt->maxs);
		bolt->owner = self;
		bolt->nextthink = level.time + 2;
		bolt->think = G_FreeEdict;
		if (hyper)
			bolt->spawnflags = 1;

		/* *** FREEZER *** */
		if (self->client && self->client->resp.freezer && self->client->pers.weapon==FindItem("hyperblaster"))
		{
			bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
			bolt->s.modelindex = gi.modelindex ("models/objects/freezer/tris.md2");
			bolt->dmg = FREEZER_DAMAGE;
			bolt->touch = freezer_touch;
			bolt->classname = "freezebolt";
		}
		/* *** DISRUPTOR RIFLE *** */
		else if (self->client && self->client->resp.disruptor && self->client->pers.weapon==FindItem("hyperblaster"))
		{
			bolt->s.sound = gi.soundindex ("misc/drfly.wav");
			bolt->s.modelindex = gi.modelindex ("models/objects/drbolt/tris.md2");
			bolt->dmg = DISRUPTOR_DAMAGE;
			bolt->touch = disruptor_touch;
			bolt->classname = "drbolt";

			bolt->s.effects=EF_TELEPORTER|EF_FLAG1;
			bolt->s.renderfx=RF_GLOW;
			bolt->movetype=MOVETYPE_FLYRICOCHET;

			if (!((int)dmflags->value&DF_INFINITE_AMMO))
				self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))]-=(DISRUPTOR_CELLS-1);

			bolt->think=disruptor_think;
			bolt->nextthink=0.1;
		}
		else
		{
			bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
			bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
			bolt->dmg = damage;
			bolt->touch = blaster_touch;
			bolt->classname = "bolt";
		}
		/* *** */

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
	/* *** LASERBLASTER *** */
	else
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
		// send laser beam temp entity to clients
		VectorCopy (tr.endpos, from);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);

		if ((tr.ent != self) && (tr.ent->takedamage))
			T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal,
				(float)damage*(float)0.75, 0, 0, MOD_LASERBLASTER);
		else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
		{
			// hit a brush, send clients
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
		/* *** TOM: BUGFIX *** */
		if (points<0)
			points=0;
		/* *** */
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;

		if (Q_stricmp(ent->classname,"pipebomb")==0)
			mod = MOD_PIPEBOMB;
		else if (Q_stricmp(ent->classname,"proximity")==0)
			mod = MOD_PROXIMITY;
		else if (Q_stricmp(ent->classname,"cluster")==0)
			mod = MOD_CLUSTER;
		else if (Q_stricmp(ent->classname,"clusterbig")==0)
			mod = MOD_CLUSTER;

		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;

	if (Q_stricmp(ent->classname,"pipebomb")==0)
		mod = MOD_PIPEBOMB;
	else if (Q_stricmp(ent->classname,"proximity")==0)
		mod = MOD_PROXIMITY;
	else if (Q_stricmp(ent->classname,"cluster")==0)
		mod = MOD_CLUSTER;

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

	/* *** CLUSTER GRENADES *** */
	if (Q_stricmp(ent->classname,"clusterbig")==0)
		Cluster_Explode (ent);
	else
	/* *** */
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

	/* *** FLARES *** */
	if (self->client && self->client->resp.flare_state)
	{
		VectorCopy (start, grenade->s.origin);
		VectorScale (aimdir, speed, grenade->velocity);
		VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
		VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
		VectorSet (grenade->avelocity, 300, 300, 300);
		grenade->movetype = MOVETYPE_BOUNCE;
		grenade->clipmask = MASK_SHOT;
		grenade->solid = SOLID_BBOX;
		grenade->s.effects |= EF_BLASTER;
		grenade->s.renderfx |= RF_SHELL_GREEN;
		VectorClear (grenade->mins);
		VectorClear (grenade->maxs);
		grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
		grenade->owner = self;
		grenade->touch = Grenade_Touch;
		grenade->nextthink = level.time + timer + FLARE_TIME;
		grenade->think = Grenade_Explode;
		grenade->dmg = FLARE_DAMAGE;
		grenade->dmg_radius = FLARE_DAMAGE_RADIUS;
		grenade->classname = "grenade";
		VectorSet(grenade->mins, -3, -3, 0);
		VectorSet(grenade->maxs, 3, 3, 6);
		grenade->mass = 2;
		grenade->health = 1;
		grenade->die = (void *)killflare;
		grenade->takedamage = DAMAGE_YES;
		grenade->monsterinfo.aiflags = AI_NOSTEP;
	}
	else
	{
	/* *** */
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
		/* *** CLUSTER GRENADES *** */
		if (self->client && self->client->resp.cluster_state)
		{
			grenade->think = Cluster_Explode;
			grenade->classname = "clusterbig";
			grenade->dmg = damage;
			grenade->dmg_radius = damage_radius;
		}
		else
		/* *** */
		{
			grenade->think = Grenade_Explode;
			grenade->classname = "grenade";
			grenade->dmg = damage;
			grenade->dmg_radius = damage_radius;
		}
	}

	gi.linkentity (grenade);
}

/* *** TOM: Changed qboolean held to int held *** */
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int held)
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
	grenade->s.effects |= EF_GRENADE;
	grenade->owner = self;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);

	/* *** PIPEBOMBS and PROXIMITY MINES *** */
	if (!held && self->client && self->client->resp.pipes)
	{
		grenade -> solid		= SOLID_TRIGGER;
		grenade -> takedamage	= DAMAGE_YES;
		grenade -> health		= 1;
		grenade -> max_health	= 1;
		grenade -> die			= killpipe;
		grenade -> parent		= self;
		grenade -> s.modelindex	= gi.modelindex ("models/objects/pipebomb/tris.md2");
		grenade -> classname	= "pipebomb";
		grenade -> dmg			= PIPEBOMB_DAMAGE;
		grenade -> dmg_radius	= PIPEBOMB_DAMAGE_RADIUS;
		grenade -> touch		= PipeBomb_Touch;
	}
	else if (!held && self->client && self->client->resp.proximity)
	{
		grenade -> solid		= SOLID_TRIGGER;
		grenade -> takedamage	= DAMAGE_YES;
		grenade -> health		= 1;
		grenade -> max_health	= 1;
		grenade -> die			= killprox;
		grenade -> parent		= self;
		grenade -> s.modelindex	= gi.modelindex ("models/objects/proximity/tris.md2");
		grenade -> classname	= "proximity";
		grenade -> nextthink	= level.time + 0.1;
		grenade -> think		= proximity_think;
		grenade -> dmg			= PROXIMITY_DAMAGE;
		grenade -> dmg_radius	= PROXIMITY_DAMAGE_RADIUS;
		grenade -> touch		= Proximity_Touch;
	}
	/* *** */
	else
	{
		grenade->solid = SOLID_BBOX;
		grenade -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
		grenade -> touch		= Grenade_Touch;
		grenade -> nextthink	= level.time + timer;
		grenade -> think		= Grenade_Explode;
		/* *** CLUSTER GRENADES *** */
		if (held!=2)
			grenade -> classname	= "hgrenade";
		else
			grenade -> classname	= "cluster";
		/* *** */
		grenade -> s.sound		= gi.soundindex("weapons/hgrenc1b.wav");
		grenade->dmg = damage;
		grenade->dmg_radius = damage_radius;
	}

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
	}
}


/*
=================


fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t			origin;
	int				n;
	unsigned int	mod;

	if (other == ent->owner)
		return;

	/* *** GUIDED MISSILES *** */
   	if (ent->owner && ent->owner->client && ent->owner->client->missile && ent->owner->client->missile==ent)
    {
        ent->owner->client->missile->takedamage=DAMAGE_NO;
   		ent->owner->client->rocketview = false;
    	ent->owner->client->missile = NULL;

        if (ent->owner->client->pers.weapon)
            ent->owner->client->ps.gunindex = gi.modelindex(ent->owner->client->pers.weapon->view_model);
    }
	/* *** */

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	/* *** BOUNCE MISSILES *** */
	if (Q_stricmp(ent->classname,"bouncerocket")==0 && !other->takedamage)
		return;
	/* *** */

	/* *** HOMING MISSILES and NUKES and AIRSTRIKES and BOUNCE MISSILES and GUIDED MISSILES *** */
	if (Q_stricmp(ent->classname,"homing")==0)
		mod = MOD_HOMING;
	else if (Q_stricmp(ent->classname,"nuke")==0)
		mod = MOD_NUKE;
	else if (Q_stricmp(ent->classname,"airstrike")==0)
		mod = MOD_AIRSTRIKE;
	else if (Q_stricmp(ent->classname,"bouncerocket")==0)
		mod = MOD_BOUNCE;
	else if (Q_stricmp(ent->classname,"guided")==0)
		mod = MOD_GUIDED;
	else
		mod = MOD_ROCKET;

	/* *** NUKES *** */
	if (other->client && Q_stricmp(ent->classname,"nuke")==0)
		other->client->nukeblind=NUKE_BLINDTIME;
	/* *** */

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, mod);
	}
	/* *** */
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

	if (Q_stricmp(ent->classname,"rocket")==0)
		mod=MOD_R_SPLASH;

	/* *** HOMING MISSILES and NUKES and AIRSTRIKES *** */
	if (mod==MOD_NUKE)
		Nuke_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, mod);
	else
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, mod);
	/* *** */

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
	if (self->client && self->client->resp.nuke_state)
		VectorScale (dir, NUKE_SPEED, rocket->velocity);
	else if (self->client && self->client->resp.guided)
		VectorScale (dir, GUIDED_SPEED, rocket->velocity);
	else
		VectorScale (dir, speed, rocket->velocity);

	if (self->client && self->client->resp.bounce)
		rocket->movetype = MOVETYPE_FLYRICOCHET;
	else
		rocket->movetype = MOVETYPE_FLYMISSILE;

	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = self;
	rocket->touch = rocket_touch;

	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	/* *** BOUNCE ROCKETS *** */
	if (self->client && self->client->resp.bounce)
		rocket->nextthink = level.time + BOUNCE_STAYALIVE;
	else
	/* *** */
		rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	/* *** HOMING MISSILES and NUKES and BOUNCE ROCKETS and GUIDED ROCKETS *** */
	if (self->client && self->client->resp.homing_state)
	{
		rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
		rocket->classname = "homing";

		if (!((int)dmflags->value&DF_INFINITE_AMMO))
			self->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] -= (HOMING_ROCKETS-1);

		rocket->think = homing_think;
		rocket->nextthink = 0.1;
	}
	else if (self->client && self->client->resp.nuke_state)
	{
		rocket->s.modelindex = gi.modelindex ("models/objects/nuke/tris.md2");
		rocket->classname = "nuke";

		if (!((int)dmflags->value&DF_INFINITE_AMMO))
			self->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] -= (NUKE_ROCKETS-1);

		rocket->dmg = NUKE_DAMAGE;
		rocket->radius_dmg = NUKE_DAMAGE_RADIUS;
		rocket->dmg_radius = NUKE_DAMAGE_RADIUS;

        rocket->takedamage = DAMAGE_YES;
        rocket->health=10;
        rocket->max_health=10;
        rocket->die=(void *)nuke_die;
        VectorSet(rocket->mins, -40, -5, -5); // size of bbox for touch
        VectorSet(rocket->maxs, -5, 5, 5); // size of bbox for touch
	}
	else if (self->client && self->client->resp.bounce)
	{
		rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
		rocket->classname = "bouncerocket";

		if (!((int)dmflags->value&DF_INFINITE_AMMO))
			self->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] -= (BOUNCE_ROCKETS-1);
	}
	else if (self->client && self->client->resp.guided)
	{
		rocket->s.modelindex = gi.modelindex ("models/objects/guided/tris.md2");
		rocket->classname = "guided";

		if (!((int)dmflags->value&DF_INFINITE_AMMO))
			self->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] -= (GUIDED_ROCKETS-1);

        rocket->takedamage = DAMAGE_YES;
        rocket->health=10;
        rocket->max_health=10;
        rocket->die=(void *)guided_die;
        // (-78.92, -9.987, -10.72), (-10.17, 9.719, 8.75)
//        VectorSet(rocket->mins, -78.92, -9.987, -10.72); // size of bbox for touch
//        VectorSet(rocket->maxs, -10.17, 9.719, 8.75); // size of bbox for touch
        VectorSet(rocket->mins, -40, -5, -5); // size of bbox for touch
        VectorSet(rocket->maxs, -5, 5, 5); // size of bbox for touch
	}
	else
	{
		rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	}
	/* *** */

	/* *** AIRSTRIKE *** */
	if (self->client && self->client->airstriking)
		rocket->classname="airstrike";

	if (self->client)
	{
		check_dodge (self, rocket->s.origin, dir, speed);
		/* *** GUIDED MISSILES *** */
		if (self->client->resp.guided)
		{
			VectorScale (dir, 450.0, rocket->velocity);
			rocket->nextthink = level.time + 0.1;
			rocket->think = guideThink;
		}
		/* *** */
	}

	gi.linkentity (rocket);

	/* *** GUIDED MISSILES *** */
	if (self->client && self->client->resp.guided)
	{
		self->client->rocketview = true;
		self->client->missile = rocket;
		self->client->was_guided = 1;
		self->client->ps.gunindex = 0;
	}
	/* *** */
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

	/* *** SNIPER RIFLE *** */
	if ((!((int)dmflags->value&DF_INFINITE_AMMO)) && self->client && self->client->resp.sniper)
		self->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] -= (SNIPER_SLUGS-1);
	/* *** */

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
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)
            /* *** TOM *** */ || Q_stricmp(tr.ent->classname,"guided")==0
                              || Q_stricmp(tr.ent->classname,"nuke")==0
                             || Q_stricmp(tr.ent->classname,"LaserDrone")==0
                             || Q_stricmp(tr.ent->classname,"ChamberPipe")==0
           /* *** */)
 				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
			{
				/* *** SNIPER RIFLE *** */
				if (self->client && self->client->resp.sniper)
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, SNIPER_DAMAGE, SNIPER_KICK, 0, MOD_SNIPER);
				else
				/* *** */
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
			}
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	/* *** SNIPER RIFLE *** */
	if (self->client && self->client->resp.sniper)
		gi.WriteByte (TE_BUBBLETRAIL);
	else
	/* *** */
		gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		/* *** SNIPER RIFLE *** */
		if (self->client && self->client->resp.sniper)
			gi.WriteByte (TE_BUBBLETRAIL);
		else
		/* *** */
			gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	/* *** SNIPER RIFLE *** */
	if (self->client && self->client->resp.sniper)
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/sniper.wav"), 1, ATTN_NORM, 0);
	else
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

//ZOID
		//don't target players in CTF
		if ((int)ctf->value && ent->client &&
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

