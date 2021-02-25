#include "g_local.h"
#include "botsutil.h"
#include "botsteam.h"

// BEGIN Teamplay
static void proxim_think (edict_t *ent);
//END Teamplay

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

// BEGIN Teamplay
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	trace_t     tr;
	vec3_t      from;
	vec3_t      end;
	qboolean poisoned;

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
	
	poisoned = false;
	if (self->client)
	{
		if ((self->client->pers.player_class == 8) && (self->client->special2))
		{
			if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= 5)
			{
				self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 5;
				gi.WriteByte (TE_BFG_LASER);      
				poisoned = true;
			}
			else
				gi.WriteByte (TE_BUBBLETRAIL);      
		}
		else
			gi.WriteByte (TE_BUBBLETRAIL);      
	}
	else
		gi.WriteByte (TE_BUBBLETRAIL);      

	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	if ((tr.ent != self) && (tr.ent->takedamage))
	{
		if (hyper)
			T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_HYPERBLASTER);
		else
			T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, 0, MOD_BLASTER);

		if (poisoned)
		{
			if (tr.ent->client)
			{
				if (tr.ent->client->pers.team != self->client->pers.team)
				{
					if (!tr.ent->client->poisontimer)
						gsutil_centerprint(tr.ent, "%s", "You have been poisoned!\n");
					tr.ent->client->poisontimer = level.time + 5.0;
				}
			}
		}
	}
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

void fire_blasterx (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;
	qboolean poisoned;	// BEGIN Teamplay END

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
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

	// BEGIN Teamplay
	poisoned = false;
	if (self->client)
	{
		if ((self->client->pers.player_class == 8) && (self->client->special2))
		{
			if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= 5)
			{
				self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 5;
				poisoned = true;
				bolt->s.effects |= EF_BLASTER;
			}
			else
				bolt->s.effects |= effect;
		}
		else
			bolt->s.effects |= effect;
		check_dodge (self, bolt->s.origin, dir, speed);
	}
	else
		bolt->s.effects |= effect;
	// END Teamplay

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	
// END Teamplay


/*
=================
fire_grenade
=================
*/

// BEGIN Teamplay
//static 
// END Teamplay
void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t		*grenade=NULL;
	int			mod;
	float		timer; 

	// BEGIN Teamplay
	if (ent->owner)
	{
		if (ent->owner->client)
			PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

	if ((Q_stricmp (ent->classname, "proximity_grenade") == 0) || 
		(Q_stricmp (ent->classname, "decoy_grenade") == 0))
	{
		if (ent->owner)
		{
			if (ent->owner->client)
			{
				if (ent->owner->client->proxdet > 0)
					ent->owner->client->proxdet--;
				else
					ent->owner->client->proxdet = 0;
			}
		}
	}

	if (Q_stricmp (ent->classname, "decoypromo") == 0)
	{
		if (ent->owner)
		{
			if (ent->owner->client)
				ent->owner->client->decoypromo = false;
		}
	}
	// END Teamplay
	
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

		if (Q_stricmp (ent->classname, "decoypromo") == 0)
			mod = MOD_DECOYPROMO;
		else if (ent->spawnflags & 1)
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

	// BEGIN Teamplay
	VectorCopy(ent->s.origin, origin);

	G_FreeEdict (ent);

	timer = 0.5;
	while ((grenade = findradius(grenade, origin, 100)) != NULL)
	{
		if ((Q_stricmp (grenade->classname, "proximity_grenade") == 0) || (Q_stricmp (grenade->classname, "decoy_grenade") == 0))
		{
			grenade->nextthink = level.time + timer;
			grenade->think = Grenade_Explode;
			timer += 0.2;
		}
	}
	// END Teamplay
}


// BEGIN Teamplay
void Cluster_Explode (edict_t *ent)
{
	vec3_t		origin;

	//Sean added these 4 vectors
	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;

	if (ent->owner)
	{
		if (ent->owner->client)
			PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_CLUSTERGREN);

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
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// SumFuka did this bit : give grenades up/outwards velocities
	VectorSet(grenade1,20,20,40);
	VectorSet(grenade2,20,-20,40);
	VectorSet(grenade3,-20,20,40);
	VectorSet(grenade4,-20,-20,40);

	// Sean : explode the four grenades outwards
	fire_grenade2(ent, origin, grenade1, 120, 10, (rand() & 4) + 1.0, 120, false);
	fire_grenade2(ent, origin, grenade2, 120, 10, (rand() & 4) + 1.0, 120, false);
	fire_grenade2(ent, origin, grenade3, 120, 10, (rand() & 4) + 1.0, 120, false);
	fire_grenade2(ent, origin, grenade4, 120, 10, (rand() & 4) + 1.0, 120, false);

	G_FreeEdict (ent);
}
// END Teamplay

#define	BLIND_FLASH		50      // Time of blindness in FRAMES

// BEGIN Teamplay
void AmmoBlock_Explode(edict_t *ent)
{
//	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
//		SetRespawn (ent, 60);
}


void Flare_Explode (edict_t *ent)
{
	if (ent->owner)
	{
		if (ent->owner->client)
		{
			if (ent->owner->client->proxdet > 0)
				ent->owner->client->proxdet--;
			else
				ent->owner->client->proxdet = 0;
		}
	}

	G_FreeEdict(ent);
}

	
void Flash_Explode (edict_t *ent)
{
	vec3_t	offset;
	edict_t	*target;
	int		flashradius=200;

	// Move it off the ground so people are sure to see it
	VectorSet(offset, 0, 0, 10);    
	VectorAdd(ent->s.origin, offset, ent->s.origin);

	if (ent->owner)
	{
		if (ent->owner->client)
		{
			PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

			if (ent->owner->client->pers.classlevel > 1)
				flashradius += 400;
		}
	}
	
	target = NULL;
	while ((target = findradius(target, ent->s.origin, flashradius)) != NULL)
	{
		if (!target->client)
				continue;       // It's not a player
		if (target == ent->owner)
				continue;       // You know when to close your eyes, don't you?
		if (target->client->pers.team == ent->owner->client->pers.team)
				continue;       // On the same team
		if (!visible(ent, target))
				continue;       // The grenade can't see it
		if (ent->owner)
		{
			if (ent->owner->client)
			{
				if (ent->owner->client->pers.classlevel < 2)
				{
					if (!infront(target, ent))
							continue;       // It's not facing it
				}
			}
		}
		
		// Increment the blindness counter
		target->client->blindTime += BLIND_FLASH * 1.05;
		target->client->blindBase = BLIND_FLASH;
		target->s.angles[YAW] = (rand() % 360); // Whee!

		// Let the player know what just happened
		// (It's just as well, he won't see the message immediately!)
		gi.cprintf(target, PRINT_HIGH, "You are blinded by a flash grenade!!!\n");

		// Let the owner of the grenade know it worked
		gi.cprintf(ent->owner, PRINT_HIGH, 
			"%s is blinded by your flash grenade!\n",
			target->client->pers.netname);
	}

	// Blow up the grenade
	BecomeExplosion1(ent);
}
// END Teamplay


// BEGIN Teamplay
//static 
// END Teamplay
void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (Q_stricmp (ent->classname, "detpipe") == 0)
		return;

	if ((Q_stricmp (ent->classname, "flare") == 0) && (other->takedamage))
	{
		if (ent->owner)
		{
			if (ent->owner->client)
				ent->owner->client->proxdet--;
		}

		G_FreeEdict(ent);
		return;
	}

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
			// BEGIN Teamplay
			if ((Q_stricmp (ent->classname, "proximity_grenade") == 0) && (Q_stricmp (other->classname, "worldspawn") == 0))
			{
				ent->movetype = MOVETYPE_NONE;
				if (random() > 0.5)
					gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
			}
			else if ((Q_stricmp (ent->classname, "flare") == 0) && (Q_stricmp (other->classname, "worldspawn") == 0))
				ent->movetype = MOVETYPE_NONE;
			// END Teamplay
		}
		else
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);

		return;
	}

	ent->enemy = other;
	// BEGIN Teamplay
	if (Q_stricmp (ent->classname, "flash_grenade") == 0)
		Flash_Explode (ent);    
	else if (Q_stricmp (ent->classname, "flare") == 0)
		Flare_Explode (ent);    
	else if (Q_stricmp (ent->classname, "cluster_grenade") == 0)
		Cluster_Explode (ent);    
	else
		Grenade_Explode (ent);
	// END Teamplay
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

	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	// BEGIN Teamplay
	if (teamplay == 0)
	{
		grenade->nextthink = level.time + timer;
		grenade->think = Grenade_Explode;
	}
	else
	{
		if ((self->client->pers.player_class == 7) && (self->client->proxdet < 6) && (self->client->weapontoggle))
		{
			grenade->classname = "detpipe";
			grenade->think = Grenade_Explode;
			grenade->nextthink = 0;
			self->client->proxdet++;
		}
		else if ((self->client->pers.player_class == 7) && (self->client->pers.classlevel > 1) && (self->client->clustermode))
		{
			grenade->classname = "cluster_grenade";
			grenade->nextthink = level.time + timer;
			grenade->think = Cluster_Explode;
		}
		else
		{
			grenade->nextthink = level.time + timer;
			grenade->think = Grenade_Explode;
		}
	}
	// END Teamplay

	gi.linkentity (grenade);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;
	int		i;

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
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 2;	// ERROR: used to be 3
	else
		grenade->spawnflags = 1;
	// BEGIN Teamplay
	if ((teamplay == 0) || (!self->client))
	{
		grenade->nextthink = level.time + timer;
		grenade->think = Grenade_Explode;
		grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
	}
	else
	{
		if ( ((self->client->pers.player_class == 1) || (self->client->pers.player_class == 2)) && (self->client->proxdet < 6)  && (self->client->weapontoggle))
		{
			grenade->nextthink = level.time + .3;
			grenade->think = proxim_think;
			if ((self->client->pers.player_class == 1) && (self->client->pers.classlevel > 2))
				grenade->delay = level.time + 240;
			else
				grenade->delay = level.time + 60;
			grenade->movetype = MOVETYPE_STOP | MOVETYPE_TOSS;
			grenade->classname = "proximity_grenade";
			self->client->proxdet++;
			grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
		}
		else if ((self->client->pers.player_class == 8)  && (self->client->weapontoggle))
		{
			grenade->think = Flash_Explode;
			grenade->classname = "flash_grenade";
			grenade->nextthink = level.time + 1.0;
			grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
		}
		else if ((self->client->pers.player_class == 6) && (self->client->proxdet < 4)  && (self->client->weapontoggle))
		{
			grenade->movetype = MOVETYPE_STOP | MOVETYPE_TOSS;
			grenade->nextthink = level.time + .3;
			grenade->think = proxim_think;
			grenade->delay = level.time + 30;
			grenade->classname = "decoy_grenade";
			self->client->proxdet++;

			i = rand() & 5;
			if (i == 1)
				grenade->s.modelindex = gi.modelindex ("models/weapons/g_bfg/tris.md2");
			else if (i == 2)
				grenade->s.modelindex = gi.modelindex ("models/weapons/g_chain/tris.md2");
			else if (i == 3)
				grenade->s.modelindex = gi.modelindex ("models/weapons/g_hyperb/tris.md2");
			else if (i == 4)
				grenade->s.modelindex = gi.modelindex ("models/weapons/g_rail/tris.md2");
			else
				grenade->s.modelindex = gi.modelindex ("models/weapons/g_rocket/tris.md2");
			
			grenade->s.effects = 0;
			grenade->s.effects |= EF_ROTATE;
		}
		else
		{
			grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
			grenade->nextthink = level.time + timer;
			grenade->think = Grenade_Explode;
		}
	}
	// END Teamplay

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
	edict_t		*grenade=NULL;
	float		timer;

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

	// BEGIN Teamplay
	VectorCopy(ent->s.origin, origin);

	if (ent->owner->client)
	{
		if (ent->owner->client->pers.playmode == 1)
		{
			timer = 0.5;
			while ((grenade = findradius(grenade, origin, 100)) != NULL)
			{
				if ((Q_stricmp (grenade->classname, "proximity_grenade") == 0) || (Q_stricmp (grenade->classname, "decoy_grenade") == 0))
				{
					grenade->nextthink = level.time + timer;
					grenade->think = Grenade_Explode;
					timer += 0.2;
				}
			}
		}
	}
	// END Teamplay

	G_FreeEdict (ent);
}

// BEGIN Teamplay
/*
	ALL NEW FUNCTIONS STARTING FROM HERE

	==================
	=fire_splitrocket=
	==================

	self, start, dir, speed (same as fire_rocket)
	side (horizontal offset)
	vert (vertical offset)
	ef (rendering effect (check q_shared.h))
*/
void fire_splitrocket (edict_t *self, vec3_t start, vec3_t dir, int speed, int side, int vert, int ef)
{
	vec3_t forward, right, up;
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed * 3, rocket->velocity);
	AngleVectors (self->s.angles, forward, right, up);
	VectorMA (rocket->velocity, side, right, rocket->velocity);	// These affect velo
	VectorMA (rocket->velocity, vert, up, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects = 0;	// Less obstruction and lag
	rocket->classname = "rocket";
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self->owner;	// Make sure its the right owner!
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 10;
	rocket->think = G_FreeEdict;
	rocket->dmg = 55;	// Normal is ~120
	rocket->radius_dmg = 55;
	rocket->dmg_radius = 55;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Split (edict_t *self)	// Called after 1/2 a sec
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	dir;

	AngleVectors (self->s.angles, forward, right, NULL);

	VectorCopy (forward, dir);
	VectorCopy (self->s.origin, start);	// This took me 1/2 an hour to figure out!!
	VectorNormalize (dir);

	// Feel free to add a dozen more! :)
	if (self->owner->client)
	{
		if (self->owner->client->splitmode == 1)
		{
			fire_splitrocket (self, start, dir, 550, 25, -20, RF_SHELL_BLUE);
			fire_splitrocket (self, start, dir, 550, -25, -20, RF_SHELL_GREEN);
//			fire_splitrocket (self, start, dir, 550, 5, 20, RF_SHELL_RED);
//			fire_splitrocket (self, start, dir, 550, -5, 20, RF_SHELL_RED);
		}
		else if (self->owner->client->splitmode == 2)
		{
			fire_splitrocket (self, start, dir, 550, 55, -20, RF_SHELL_BLUE);
			fire_splitrocket (self, start, dir, 550, -55, -20, RF_SHELL_GREEN);
//			fire_splitrocket (self, start, dir, 550, 25, 20, RF_SHELL_RED);
//			fire_splitrocket (self, start, dir, 550, -25, 20, RF_SHELL_RED);
		}
		else
		{
			fire_splitrocket (self, start, dir, 550, 110, -20, RF_SHELL_BLUE);
			fire_splitrocket (self, start, dir, 550, -110, -20, RF_SHELL_GREEN);
//			fire_splitrocket (self, start, dir, 550, 55, 20, RF_SHELL_RED);
//			fire_splitrocket (self, start, dir, 550, -55, 20, RF_SHELL_RED);
		}
	}
	else
	{
		fire_splitrocket (self, start, dir, 550, 110, -20, RF_SHELL_BLUE);
		fire_splitrocket (self, start, dir, 550, -110, -20, RF_SHELL_GREEN);
//		fire_splitrocket (self, start, dir, 550, 55, 20, RF_SHELL_RED);
//		fire_splitrocket (self, start, dir, 550, -55, 20, RF_SHELL_RED);
	}

//	gi.WriteByte (svc_temp_entity);	// Explode
//	if (self->waterlevel)
//		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
//	else
//		gi.WriteByte (TE_ROCKET_EXPLOSION);
//	gi.WritePosition (self->s.origin);
//	gi.multicast (self->s.origin, MULTICAST_PVS);
//
//	G_FreeEdict(self);	// FIXME: T_RadiusDamage will crash Q2!
}

// END Teamplay

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	// BEGIN Teamplay
	VectorScale (dir, speed * 3, rocket->velocity);
	// END Teamplay
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;

	// BEGIN Teamplay
//	rocket->s.effects |= EF_ROCKET;
	rocket->s.effects = 0;
	// END Teamplay

	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;

	// BEGIN Teamplay
	if (!self->client)
	{
		rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
	}
	else
	{
		if ((self->client->pers.classlevel > 1) && (self->client->weapontoggle))
		{
			rocket->nextthink = level.time + 0.1;
			rocket->think = Split;
		}
		else
		{
			rocket->nextthink = level.time + 8000/speed;
			rocket->think = G_FreeEdict;
		}
	}
	// END Teamplay

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

	// BEGIN Teamplay
	if (!self->client)
		mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	else
	{
		if (self->client->pers.classlevel < 3)
			mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
		else
			mask = CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEADMONSTER;
	}
	// END Teamplay

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

	// BEGIN Teamplay
	// send gun puff / flash
	if (self->client->pers.classlevel < 3)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
//		gi.multicast (start, MULTICAST_PHS);
		if (water)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos);
			gi.multicast (tr.endpos, MULTICAST_PHS);
		}
	}
	// END Teamplay

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

			// BEGIN Teamplay
			if(OnSameTeam(ent, self->owner))
				continue;
			// END Teamplay

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

		// BEGIN Teamplay
		if(OnSameTeam(ent, self->owner))
			continue;
		// END Teamplay

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


// BEGIN Teamplay
static void proxim_think (edict_t *ent)
{
	edict_t *blip = NULL;

	if (level.time > ent->delay)
	{
//		if (ent->owner)
//		{
//			if (ent->owner->client->proxdet > 0)
//				ent->owner->client->proxdet--;
//			else
//				ent->owner->client->proxdet = 0;
//		}
		Grenade_Explode(ent);
		return;
	}
	
	ent->nextthink = level.time + .5;

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
		if (blip->client->pers.team == ent->owner->client->pers.team)
			continue;
		if ((blip->client->pers.player_class == 7) && (Q_stricmp (ent->classname, "decoy_grenade")))
		{
			if (blip->client->special2 == true)
			{
				blip->client->pausetimer = level.time;
				blip->client->thinkmsg = true;
				ent->nextthink = level.time + 3.0;

				if (ent->owner)
				{
					if (ent->owner->client->proxdet > 0)
						ent->owner->client->proxdet--;
					else
						ent->owner->client->proxdet = 0;
				}

				ent->owner = blip;
				ent->delay = level.time + 60;
				return;
			}
		}

//		if (ent->owner)
//		{
//			if (ent->owner->client->proxdet > 0)
//				ent->owner->client->proxdet--;
//			else
//				ent->owner->client->proxdet = 0;
//		}
		Grenade_Explode(ent);
		return;
	}
}
// END Teamplay


// BEGIN Teamplay
void light_flare (edict_t *self)
{
    self->s.effects = EF_BLASTER;
	self->nextthink = level.time + 20.0;
	self->think = Flare_Explode;
}

void fire_flare (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
    edict_t *grenade, *blip=NULL;
    vec3_t  dir;
    vec3_t  forward, right, up;
	int		li_team;

    if (!self->client)
		return;
	
	vectoangles (aimdir, dir);
    AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_STOP | MOVETYPE_TOSS;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);

	grenade->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
    grenade->owner = self;
    grenade->touch = Grenade_Touch; 
    grenade->dmg = 0;
    grenade->dmg_radius = 0;
    grenade->classname = "flare";
    VectorSet(grenade->mins, -3, -3, 0);
    VectorSet(grenade->maxs, 3, 3, 6);
	grenade->nextthink = level.time + 3.0;
	grenade->think = light_flare;
	grenade->spawnflags = 1;

	self->client->proxdet++;

    gi.linkentity (grenade);

	li_team = self->client->pers.team;
	while ((blip = findradius(blip, grenade->s.origin, 250)) != NULL)
	{
		if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if (!blip->client)
			continue;
		if (blip == grenade->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(grenade, blip))
			continue;
		if (blip->client->pers.team == li_team)
			continue;
		if (blip->client->pers.player_class == 3)
		{
			if (blip->client->invisible)
			{
				blip->client->last_movement = level.time;
				blip->client->invisible = false;
				gsutil_centerprint(blip, "%s", "You are visible again!\n");
				blip->s.modelindex = 255;
			}

			if (blip->client->quadcloak)
			{
				blip->client->quadcloak = false;
				gsutil_centerprint(blip, "%s", "You are visible again!\n");
				blip->s.modelindex = 255;
			}
		}
	}
}
// END Teamplay
