#include "g_local.h"
#include "laser.h"

void eject_shell (edict_t *self);
void Concussion_Explode (edict_t *ent);
void Plasma_Explode (entity *ent);
void Rail_Grenade_Explode (edict_t *ent);
void Smoke_Grenade_Explode (edict_t *ent);
void Cluster_Explode (edict_t *ent);
void Zylon_Grenade(edict_t *ent);
void WriteMessage (edict_t *ent, edict_t *target);

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static qboolean	is_quad;

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
bullet_trace

Used for making those cool bullets traces on walls
=================
*/
void bullet_trace (trace_t tr)
{
	edict_t *bhole;

    bhole = G_Spawn();
	bhole->s.modelindex = gi.modelindex ("models/objects/spots/spotbl.md2");
	bhole->movetype = MOVETYPE_NOCLIP;
	bhole->clipmask = MASK_SHOT;
	bhole->solid = SOLID_BBOX;
	bhole->owner = world;
	bhole->classname = "bhole";
    bhole->think = G_FreeEdict;
    bhole->nextthink = level.time + bhole_life->value;
	VectorCopy (tr.endpos, bhole->s.origin);
    vectoangles (tr.plane.normal, bhole->s.angles);
    gi.linkentity (bhole);
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
					G_SplashEntity (TE_SPLASH, 8, tr.endpos, tr.plane.normal, color, MULTICAST_PVS);

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
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
                    G_ImpactEntity (te_impact, tr.endpos, tr.plane.normal, MULTICAST_PVS);

					if (bhole_life->value > 0)
						bullet_trace (tr);

					if (G_ClientExists(self))
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

        G_LineEntity (TE_BUBBLETRAIL, water_start, tr.endpos, tr.endpos, MULTICAST_PVS);
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
    eject_shell (self);
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
	{
	    fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
		eject_shell (self);
	}
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
		else if (strcmp (self->classname , "fbolt"))
			mod = MOD_BOLT;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		if (!plane)
			G_ImpactEntity(TE_BLASTER, self->s.origin, vec3_origin, MULTICAST_PVS);
		else
			G_ImpactEntity(TE_BLASTER, self->s.origin, plane->normal, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
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

	if (G_ClientExists(self))
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

void fire_laser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int line)
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

   if (line)
       G_LineEntity (line, start, tr.endpos, self->s.origin, MULTICAST_PHS);

   if ((tr.ent != self) && (tr.ent->takedamage))
	   T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_HYPERBLASTER);
   else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
   {
	     // hit a brush, send clients 
         // a light flash and sparks temp entity.
         G_ImpactEntity (TE_BLASTER2, tr.endpos, dir, MULTICAST_PVS);
   }
}       

void Pipebomb_Explode (edict_t *ent)
{
	vec3_t		origin;

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
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_PIPEBOMB);
	}

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_PIPEBOMB);

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
		{
			if (ent->owner->playerclass == 2 || ent->owner->client->resp.it == 2)
				gi.WriteByte (TE_EXPLOSION1_NP);
			else
				gi.WriteByte (TE_GRENADE_EXPLOSION);
		}
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Sensor_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	ent->think (ent);
}

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

	if (ent->owner->playerclass == 2)
		mod = MOD_ZYLON_GAS;

	if (ent->touch == Sensor_Touch)
		mod = MOD_SENSOR_GRENADES;

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
		{
			if (ent->owner->playerclass == 2 || ent->owner->client->resp.it == 2)
				gi.WriteByte (TE_EXPLOSION1_NP);
			else 
				gi.WriteByte (TE_GRENADE_EXPLOSION);
		}
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void Tag_Think (edict_t *ent)
{
	vec3_t temp;

	// owner disconnected
	if (!G_ClientExists(ent->owner))
	{
		G_FreeEdict (ent);
		return;
	}

    // enemy disconnected
	if (!G_ClientExists(ent->enemy))
	{
		G_FreeEdict (ent);
		return;
	}

	// already have one grenade
	if (G_EntExists(ent->tag_gr))
	{
		G_FreeEdict (ent);
		return;
	}

	// he is dead!
	if (!G_ClientNotDead(ent->enemy))
	{
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (ent->enemy->s.origin, temp);
	temp[2] += (ent->enemy->viewheight + 10);

	VectorCopy (temp, ent->s.origin);

	ent->delay += 1;
	ent->nextthink = level.time + .1;

	if (ent->delay == 30)
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NONE, 0 );
		ent->delay = 0;
	}
}

static void Tag_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}


	if (ent->spawnflags & 1)
	{
		if (random() > 0.5)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
	}
	else
   		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);

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
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);

		return;
	}

	other->tag_gr = ent;

	ent->enemy = other;
	ent->think = Tag_Think;
	Tag_Think (ent); // why explode ?
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


	if (ent->owner->playerclass == 3 || ent->owner->client->resp.it == 3 || ent->classname == "pipebomb")
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
      		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);

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
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		return;
	}

	ent->enemy = other;
	ent->think (ent); // why explode ?
}

static void Pipebomb_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}


	if (ent->spawnflags & 1)
	{
		if (random() > 0.5)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
	}
	else
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
}

//===============code change==============
qboolean onground(edict_t *ent)
{
    vec3_t          point;
    trace_t         trace;

    if (ent->velocity[2])
        return false;

	VectorCopy (ent->s.origin, point);
	point[2] -= .25;

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_SHOT);

	if (trace.endpos == point)
		return false;

    if (trace.plane.normal[2] < 0.7 && !trace.startsolid)
		return false;

	return true;
}

static void Leapfrog_Die (edict_t *ent)
{
	if(ent->wait < level.time)
		Grenade_Explode (ent);
	else
	    ent->nextthink = level.time + .1;

	ent->think = Leapfrog_Die;
}

static void Leapfrog_Think (edict_t *ent)
{
	edict_t	*target = NULL;
	edict_t *blip = NULL;
	vec3_t	targetdir, blipdir;

	if (ent->wait < level.time || !G_ClientExists (ent->owner) || ent->owner->deadflag)
	{
		ent->velocity[0] = 0;
		ent->velocity[1] = 0;
		ent->velocity[2] = 600;
		ent->wait = level.time + .2;
		Leapfrog_Die(ent);
	}

	if (!ent->s.renderfx)
		(ent->owner->client->resp.s_team == 1) ? (ent->s.renderfx = RF_SHELL_RED) : (ent->s.renderfx = RF_SHELL_BLUE);

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!G_ClientExists (blip))
			continue;
		if (blip == ent->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (blip->client->resp.s_team == ent->owner->client->resp.s_team)
			continue;

		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}

	if ((target != NULL) && (onground(ent)))
	{
		// target acquired, change our direction toward it

		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.2, targetdir);
		VectorNormalize(targetdir);
		vectoangles (targetdir, ent->s.angles);
		VectorScale (targetdir, 200, ent->velocity);
		ent->velocity[2] += 200;
		ent->nextthink = level.time + 0.1;
	}
	else if (onground(ent))
	{
		ent->velocity[0] = crandom()*200;
		ent->velocity[1] = crandom()*200;
		ent->velocity[2] = 200 + random()*50;
		vectoangles(ent->velocity, ent->s.angles);
		ent->nextthink = level.time + 0.5;
	}
	else
	{
		vectoangles(ent->velocity, ent->s.angles);
		ent->nextthink = level.time + 0.1;
	}
}

/*
=================
fire_grenade
=================
*/

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);

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

void fire_sgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->touch = Sensor_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
   	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

void laser_explode (edict_t *self)
{
	vec3_t v;

	if (!G_ClientExists(self->owner))
	{
		G_FreeEdict (self);
		return;
	}

	VectorSubtract (self->owner->s.origin, self->s.origin, v);
	VectorNormalize (v);

	PlaceLaser (self, v);
	self->nextthink = level.time + 20;
	self->think = G_FreeEdict;
}

void Laser_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (other->client)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);

	self->movetype = MOVETYPE_NONE;
	self->s.sound = 0;
	VectorClear (self->velocity);
	VectorClear (self->avelocity);

	gi.linkentity (self);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

    if (self->playerclass == 7)
	{
	    fire_flamegrenade2 (self, start, aimdir, damage, speed, timer, damage_radius, held);
		return;
	}

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
	grenade->nextthink = level.time + timer;

	if (self->client->gr_type)
	{
		if (self->playerclass == 1 || self->client->resp.it == 1)
			grenade->think = Rail_Grenade_Explode;
		else if (self->playerclass == 4 || self->client->resp.it == 4)
			grenade->think = Smoke_Grenade_Explode;
		else if (self->playerclass == 6 || self->client->resp.it == 6)
			grenade->think = Concussion_Explode;
		else if (self->playerclass == 5 || self->client->resp.it == 5)
			grenade->think = Cluster_Explode;
		else if (self->playerclass == 8 || self->client->resp.it == 8)
			grenade->think = Plasma_Explode;
		else if (self->playerclass == 9)
			grenade->think = G_FreeEdict;
		else if (self->playerclass == 2 || self->client->resp.it == 2)
		{
			grenade->think = Leapfrog_Think;
			grenade->wait = level.time + 20;  // after 20 seconds leapfrogs explode to reduce lag
			grenade->s.effects |= EF_COLOR_SHELL;
		}
		else
			// thief ?
 			grenade->think = laser_explode; // bounce grenades

		if (self->playerclass == 9)
     		grenade->touch = Tag_Touch;
		else if (self->playerclass == 3)
		    grenade->touch = Laser_Touch;
		else
			grenade->touch = Pipebomb_Touch;
	}
	else
	{
		grenade->think = Grenade_Explode;
    	grenade->touch = Grenade_Touch;
	}

	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
    grenade->classname = "hgrenade";

	(held) ? (grenade->spawnflags = 3) : (grenade->spawnflags = 1); 

	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		grenade->think (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

void Cmd_DetPipes_f (edict_t *ent, char *cmd)
{
	edict_t	*blip = NULL;
	int found = 0;

	if (ent->playerclass != 5 && ent->client->resp.it != 5)
		return;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!strcmp(blip->classname, "pipebomb") && blip->owner == ent)
		{
			if (strlen(cmd) > 0)
			{
				if (atoi (cmd) == found)
					break;
			}

			found ++;
			Pipebomb_Explode(blip);
		}
	}

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Pipebombs"))] += found;
}

/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);

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
    rocket->s.modelindex3 = 0;
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (G_ClientExists(self))
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
    G_LineEntity (TE_RAILTRAIL, start, tr.endpos, self->s.origin, MULTICAST_PHS);

	if (water)
        G_LineEntity (TE_RAILTRAIL, start, tr.endpos, tr.endpos, MULTICAST_PHS);
 
	if (G_ClientExists(self))
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

            G_PointEntity(TE_BFG_EXPLOSION, ent->s.origin, MULTICAST_PHS);
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

    G_PointEntity(TE_BFG_BIGEXPLOSION, self->s.origin, MULTICAST_PVS);
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

	dmg = 5;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!G_ClientExists(ent)) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		if (G_ClientExists(ent))
			if (ent->client->resp.s_team == self->ripstate)
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
                G_SplashEntity (TE_LASER_SPARKS, 4, tr.endpos, tr.plane.normal, self->s.skinnum, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

        G_LineEntity (TE_BFG_LASER, self->s.origin, tr.endpos, self->s.origin, MULTICAST_PHS);
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
	bfg->ripstate = self->client->resp.s_team;
	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (G_ClientExists(self))
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}

void heat_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (ent);
		return;
	}

	if (PBM_InWater(ent))
        {       PBM_BecomeSteam(ent);
		return;
	}

	if (!other->takedamage)
		G_FreeEdict(ent);
	else
	{
		if ((other != ent->owner) && (other->takedamage))
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 1, DAMAGE_ENERGY, 40);
	}
}

void Rail_Grenade_Explode (edict_t *ent)
{
	vec3_t		    origin;
    vec3_t          grenade_angs;
    vec3_t          forward, right, up;
    int             n;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

    // Now trace the railgun shots:
    for (n = 0; n < 10; n++)
    {
        grenade_angs[0] = -5 + (crandom() * 2);
        grenade_angs[1] = n*36 + crandom()*2;
        grenade_angs[2] = 0;
        AngleVectors (grenade_angs, forward, right, up);
        fire_rail (ent->owner, origin, forward, 100, 120);
    }

    Grenade_Explode (ent);
}

void Spawn_Smoke (edict_t *ent)
{
        vec3_t  max,v,v2;
        trace_t tr;
        int     i,counter;
        float   dist;
        float dist2;
        edict_t *target;

        ent->nextthink = level.time + .001;
        ent->bounces++;
        i = 0;
        counter = ent->bounces;

        if (counter > 20)     //we only want to go up to 20 due to overflow problems...
                counter = 20;

//Create point that is the maximum distance the smoke will travel this think...
        max[0] = -4.75 * counter + ent->s.origin[0];
        max[1] = -4.75 * counter + ent->s.origin[1];
        max[2] = -4.5 * counter + ent->s.origin[2];
        VectorSubtract (max, ent->s.origin, max);
        dist = VectorLength (max);

//Now create another "standard point" to compare the maximum to
        max[0] = -4.75 + ent->s.origin[0];
        max[1] = -4.75 + ent->s.origin[1];
        max[2] = -4.5 + ent->s.origin[2];
        VectorSubtract (max, ent->s.origin, max);
        dist = dist / VectorLength (max);

// Speed up: do we really need to and the ent->s.origin values to max? Because (1+2)-2 = 1. Do the adding and
// subtracting cancel each other out? Yup! But After I realized that I didn't really feel like fixing it, if you want to
// optimize it, feel free!

retry:
// the following code will select a random point in space a certain distance from the ent's origin
        v[0] = -4.75 * counter;
        v[1] = -4.75 * counter;
        v[2] = -4.5 * counter;
        v2[0] = 9.5 * counter;
        v2[1] = 9.5 * counter;
        v2[2] = 9 * counter;
        v[0] = v[0] + v2[0] * random();
        v[1] = v[1] + v2[1] * random();
        v[2] = v[2] + v2[2] * random();
        VectorAdd (v, ent->s.origin, v);

// If the ent can't see this point, then pick a new one...
        tr = gi.trace (ent->s.origin, NULL, NULL, v, ent, MASK_SHOT);
        if (tr.fraction == 1.0)
                i++;
        else
                goto retry;

// draw a puff of "smoke" particles at the point that was just selected
        G_PointEntity(TE_CHAINFIST_SMOKE, v, MULTICAST_PVS);

// A complicated little condition that check to see if there are any more "smoke" puffs to draw
        if (i < counter * (sqrt(1 - sqrt(dist)/dist) + 1))
                goto retry;
// Now, find the players that are near the smoke grenade and tweak their smoke value
        dist = dist * VectorLength (max);

        target = NULL;
        dist = dist * 2;
        while ((target = findradius(target, ent->s.origin, dist)) != NULL)
        {
                if (!target->client || !visible(ent,target)) // has to be a player that can see the smoke grenade
                        continue;
				if (target->client->resp.s_team == ent->owner->client->resp.s_team)
					continue;

                VectorSubtract(ent->s.origin,target->s.origin,max);
                dist2 = VectorLength (max);
                dist2 = 1 - (dist2*dist2) / (dist*dist); // makes the smoke get thicker the closer you get to it
                target->client->smoke += dist2; // add the result to the player's smoke value
        }

        if (ent->bounces > 50)
                G_FreeEdict (ent);
}

void Smoke_Grenade_Explode (edict_t *ent)
{
        vec3_t          origin;
        int                     mod;

        ent->think = Spawn_Smoke;
        ent->nextthink = level.time + .1;
        ent->bounces = 0;
        VectorCopy(vec3_origin,ent->velocity);
        ent->s.sound = 0;

        return; // we don't want the grenade to explode anymore so break out of the function

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
   // Blow up the grenade
    G_PointEntity(TE_BOSSTPORT, ent->s.origin, MULTICAST_PVS);


	G_FreeEdict (ent);
}

void fire_grenade3 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->owner = self->owner; //do not damage player
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
   	grenade->classname = "grenade";

	gi.linkentity (grenade);
}


void pipebomb_throw (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;
	vec3_t  mins = {-6, -6, -6};
	vec3_t  maxs = {6, 6, 6};

    if (self->playerclass != 5 && self->client->resp.it != 5)
		return;

	damage_radius = damage+40;
	
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
	VectorCopy (mins, grenade->mins);
	VectorCopy (maxs, grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/pipe/tris.md2");
	grenade->owner = self;
	grenade->ripstate = grenade->owner->client->resp.s_team; // no damage to it's owner in teamplay
	grenade->touch = Pipebomb_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Pipebomb_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "pipebomb";
	gi.linkentity (grenade);
}

void Cluster_Explode (edict_t *ent)
{
	vec3_t		origin;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	fire_grenade3(ent, origin, tv(20, 20, 40), 120, 10, 2.0, 120);
	fire_grenade3(ent, origin, tv(20, -20, 40), 120, 10, 2.0, 120);
	fire_grenade3(ent, origin, tv(-20, 20, 40), 120, 10, 2.0, 120);
	fire_grenade3(ent, origin, tv(-20, -20, 40), 120, 10, 2.0, 120);

    Grenade_Explode (ent);
}

void plasma_explode (edict_t *ent)
{
	ent->nextthink = level.time + FRAMETIME;
    ent->s.frame++;
    
    if (ent->s.frame == 5)
		ent->think = G_FreeEdict;
}

void Plasma_Explode (entity *ent)
{
	ent->classname = "plasma explosion";
    T_RadiusDamage(ent, ent->owner, 50, NULL, 40, MOD_PLASMA);

    // Kludge to get louder sound, since vol can't exceed 1.0
    gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
    gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
    gi.sound (ent, CHAN_ITEM, gi.soundindex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);

    ent->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
    ent->s.frame = 0;
    ent->think = plasma_explode;
    ent->nextthink = level.time + FRAMETIME;
    ent->movetype = MOVETYPE_NONE;
    ent->s.renderfx = RF_TRANSLUCENT;
	ent->s.effects = EF_BFG;

    G_PointEntity(TE_BFG_BIGEXPLOSION, ent->s.origin, MULTICAST_PVS);
	G_FreeEdict (ent);
}

