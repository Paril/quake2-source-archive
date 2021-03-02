#include "g_local.h"

void Gas_Grenade(edict_t *ent);

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
	if (CVAR_SKILL_LEVEL == 0)
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
					G_Spawn_Splash(TE_SPLASH, 8, color, tr.endpos, tr.plane.normal, tr.endpos);

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
					G_Spawn_Sparks(te_impact, tr.endpos, tr.plane.normal, tr.endpos);

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

		G_Spawn_Trails(TE_BUBBLETRAIL, water_start, tr.endpos, pos);
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
		G_Spawn_Sparks(TE_BLASTER, self->s.origin, plane->normal, self->s.origin);

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
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex (LASER_MODEL);
	bolt->s.sound = LASFLY_SOUND;
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
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);

void Teleport_Grenade(edict_t *grenade){
	edict_t *ent = NULL;
    vec3_t spawn_origin={0,0,0},
    spawn_angles={0,0,0};

    while ((ent = findradius(ent, grenade->s.origin, 300)) != NULL){
	    if (!G_EntExists(ent)) continue;

        // Have System select a random spawning location
        SelectSpawnPoint(ent, spawn_origin, spawn_angles);

        // Copy over player's last movement info.
        ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
        ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
        ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;

        // Actually Relocate player to the new spot
        VectorCopy(spawn_origin, ent->s.origin);

        // Must set Ents Respawn Time!
        ent->client->respawn_time=level.time;

        // So Particle effect at new spot
        ent->s.event = EV_PLAYER_TELEPORT;

        // play teleport sound effects.
        gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
	} // while
}

#define         FLASH_RADIUS                    500
#define         BLIND_FLASH                     100      // Time of blindness in FRAMES

void Flash_Explode (edict_t *ent)
{
	vec3_t      offset;
    edict_t *target;

    // Move it off the ground so people are sure to see it
    VectorSet(offset, 0, 0, 10);
    VectorAdd(ent->s.origin, offset, ent->s.origin);

    if (ent->owner->client)
	    PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    target = NULL;
    while ((target = findradius(target, ent->s.origin, FLASH_RADIUS)) != NULL)
    {
	    if (target == ent->owner)
	        continue;       // You know when to close your eyes, don't you?
        if (!target->client)
            continue;       // It's not a player
        if (!visible(ent, target))
            continue;       // The grenade can't see it

        // Increment the blindness counter
        target->client->blindTime += BLIND_FLASH * 1.5;
        target->client->blindBase = BLIND_FLASH;
		target->s.angles[YAW] = (rand() % 360); // Whee!
    }

    // Blow up the grenade
    BecomeExplosion1(ent);
}

void Flash_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
	    return;

    // If it goes in to orbit, it's gone...
    if (surf && (surf->flags & SURF_SKY))
    {
        G_FreeEdict (ent);
        return;
    }

    // All this does is make the bouncing noises when it hits something...
    if (!other->takedamage)
    {
	    if (random() > 0.5)
	        gi.sound (ent, CHAN_VOICE, HANDGRENADE1A_SOUND, 1, ATTN_NORM, 0);
        else
            gi.sound (ent, CHAN_VOICE, HANDGRENADE2_SOUND, 1, ATTN_NORM, 0);
        return;
	}

    // The ONLY DIFFERENCE between this and "Grenade_Touch"!!
    Flash_Explode (ent);
}

void Shrapnel_Explode (edict_t *ent)
{
	vec3_t        origin;
    vec3_t grenade1;
    vec3_t grenade2;
    vec3_t grenade3;
    vec3_t grenade4;
	vec3_t grenade5;
    vec3_t grenade6;
    vec3_t grenade7;
    vec3_t grenade8;
    int             mod;
    int             i, te_type;
    vec3_t offset;

    if (ent->owner->client)
	    PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    VectorSet(offset,0,0,32);
    VectorAdd(offset,ent->s.origin,offset);
    VectorCopy (offset, ent->s.origin);
    VectorSet(grenade1,20,20,5);
    VectorSet(grenade2,20,-20,5);
    VectorSet(grenade3,-20,20,5);
    VectorSet(grenade4,-20,-20,5);
    VectorSet(grenade5,0,20,5);
    VectorSet(grenade6,0,-20,5);
    VectorSet(grenade7,-20,0,5);
    VectorSet(grenade8,20,0,5);
    for (i = 0; i < 6; i++)
    {
	    fire_bullet (ent, offset, grenade1, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade2, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade3, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade4, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade5, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade6, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade7, 2, 2, 4500, 4500, MOD_SHRAPNEL);
        fire_bullet (ent, offset, grenade8, 2, 2, 4500, 4500, MOD_SHRAPNEL);
    }

    if (ent->takedamage)
    {
        float    points;
        vec3_t    v;
        vec3_t    dir;

        VectorAdd (ent->mins, ent->maxs, v);
        VectorMA (ent->s.origin, 0.5, v, v);
        VectorSubtract (ent->s.origin, v, v);
        points = ent->dmg - 0.5 * VectorLength (v);
        VectorSubtract (ent->s.origin, ent->s.origin, dir);
        mod = MOD_HANDGRENADE;
        T_Damage (ent, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

    mod = MOD_HG_SPLASH;
    T_RadiusDamage(ent, ent->owner, ent->dmg, ent, ent->dmg_radius, mod);

    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (ent->waterlevel) {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION_WATER;
		else
			te_type=TE_ROCKET_EXPLOSION_WATER; }
	else {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION;
		else
			te_type=TE_ROCKET_EXPLOSION; }

	G_Spawn_Explosion(te_type, origin, ent->s.origin);

    G_FreeEdict (ent);
}

static void Shrapnel_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	    if (random() > 0.5)
	        gi.sound (ent, CHAN_VOICE, HANDGRENADE1A_SOUND, 1, ATTN_NORM, 0);
        else
            gi.sound (ent, CHAN_VOICE, HANDGRENADE2_SOUND, 1, ATTN_NORM, 0);
        return;
    }

    ent->enemy = other;
    Shrapnel_Explode (ent);
}

void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod, te_type;

    if (G_EntExists(ent->owner))
	    if (ent->owner->client->recon_grenade_toggle == 2) {
	        ent->gas_timer = level.time+10+(random()*10);
            Gas_Grenade(ent);
            return; }

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
		if (ent->spawnflags & 4)
			mod=MOD_CLUSTER_BOMBS;
		else if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 4)
		mod=MOD_CLUSTER_BOMBS;
	else if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;

	if (G_EntExists(ent->owner) && ent->owner->client->teleport_toggle)
	    Teleport_Grenade(ent);
    else
		T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	if (ent->waterlevel) {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION_WATER;
		else
			te_type=TE_ROCKET_EXPLOSION_WATER; }
	else {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION;
		else
			te_type=TE_ROCKET_EXPLOSION; }

	G_Spawn_Explosion(te_type, origin, ent->s.origin);

	G_FreeEdict (ent);
}

void Pipe_Explode (edict_t *ent)
{
	vec3_t		origin;
	int	te_type;

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

	if (ent->waterlevel) {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION_WATER;
		else
			te_type=TE_ROCKET_EXPLOSION_WATER; }
	else {
		if (ent->groundentity)
			te_type=TE_GRENADE_EXPLOSION;
		else
			te_type=TE_ROCKET_EXPLOSION; }

	G_Spawn_Explosion(te_type, origin, ent->s.origin);

	G_FreeEdict (ent);
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
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, HANDGRENADE1A_SOUND, 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, HANDGRENADE2_SOUND, 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, GRENADE_BOUNCE_SOUND, 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

static void Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
    self->nextthink = level.time + .1;
    self->think = Grenade_Explode;
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
	grenade->s.modelindex = gi.modelindex (GRENADE_MODEL);
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
    VectorSet(grenade->mins, -3, -3, 0);
    VectorSet(grenade->maxs, 3, 3, 6);
    grenade->mass = 2;
    grenade->health = 10;
    grenade->die = Grenade_Die;
    grenade->takedamage = DAMAGE_YES;
    grenade->monsterinfo.aiflags = AI_NOSTEP;

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
	grenade->s.modelindex = gi.modelindex (HGRENADE_MODEL);
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	grenade->s.sound = HANDGRENADE1C_SOUND;
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
    VectorSet(grenade->mins, -3, -3, 0);
    VectorSet(grenade->maxs, 3, 3, 6);
    grenade->mass = 2;
    grenade->health = 10;
    grenade->die = Grenade_Die;
    grenade->takedamage = DAMAGE_YES;
    grenade->monsterinfo.aiflags = AI_NOSTEP;

	if (self->client)
	{
		if (self->client->pipe_toggle)
		{
			grenade->classname = "detpipe";
			grenade->think = Pipe_Explode;
			grenade->nextthink = level.time + 60;
			grenade->s.sound = 0;
		}
		else if (self->client->shrapnel_toggle)
		{
			grenade->classname = "shrapnel";
			grenade->think = Shrapnel_Explode;
			grenade->touch = Shrapnel_Touch;
			grenade->spawnflags = 4;
		}
		else if (self->client->recon_grenade_toggle == 1)
		{
            grenade->touch = Flash_Touch;
            grenade->think = Flash_Explode;
            grenade->classname = "flash_grenade";
		}
	}

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, GRENADE_TOSS_SOUND, 1, ATTN_NORM, 0);
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
		if (ent->owner->client)
		{
			if (strcmp(ent->classname, "guided") == 0)
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
			else if (strcmp(ent->classname, "homing") == 0)
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
			else
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
		}
		else
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!CVAR_DEATHMATCH)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, DEBRIS2_MODEL, 2, ent->s.origin);
			}
		}
	}

	if (ent->owner->client)
	{
		if (strcmp(ent->classname, "guided") == 0)
			T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_GUIDED);
		else if (strcmp(ent->classname, "homing") == 0)
			T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_HOMING);
		else
			T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH);
	}
	else
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH);

	if (ent->waterlevel)
		G_Spawn_Explosion(TE_ROCKET_EXPLOSION_WATER, origin, origin);
	else
		G_Spawn_Explosion(TE_ROCKET_EXPLOSION, origin, origin);

	if (ent->owner->client)
	{
		ent->owner->client->rocketview = false;
		ent->owner->client->missile = NULL;
	}

	G_FreeEdict (ent);
}

static void Rocket_Explode (edict_t *ent)
{
	vec3_t          origin;

    if (ent->owner->client)
	    PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    // calculate position for the explosion entity
    VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (strcmp(ent->classname, "guided") == 0)
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_GUIDED);
	else if (strcmp(ent->classname, "guided") == 0)
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_HOMING);
	else
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH);

	if (ent->waterlevel)
		G_Spawn_Explosion(TE_ROCKET_EXPLOSION_WATER, origin, origin);
	else
		G_Spawn_Explosion(TE_ROCKET_EXPLOSION, origin, origin);

	ent->owner->client->rocketview = false;
	ent->owner->client->missile = NULL;

    G_FreeEdict (ent);
}

static void Rocket_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
    self->nextthink = level.time + .1;
    self->think = Rocket_Explode;
}

void guide_think (edict_t *ent)
{
	vec3_t offset;
	vec3_t forward;
	vec3_t tvect;
	float dist;

	VectorSet (offset, 0, 0, ent->owner->viewheight);
	VectorAdd (ent->s.origin, offset, tvect);
	VectorSubtract (tvect, ent->s.origin, tvect);
	dist = VectorLength (tvect) + 500.0;
	AngleVectors (ent->owner->client->ps.viewangles, forward, NULL, NULL);
	VectorScale (forward, dist, forward);
	VectorAdd (forward, tvect, tvect);
	dist = VectorNormalize (tvect);
	VectorCopy (forward, ent->velocity);
	vectoangles (tvect, ent->s.angles);
	ent->nextthink = level.time + FRAMETIME;
}

void homing_think (edict_t *ent)
{
	edict_t *target = NULL;
    edict_t *blip = NULL;
    vec3_t  targetdir, blipdir;
    vec_t   speed;

    while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
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
        VectorScale(targetdir, 0.2, targetdir);
        VectorAdd(targetdir, ent->movedir, targetdir);
        VectorNormalize(targetdir);
        VectorCopy(targetdir, ent->movedir);
        vectoangles(targetdir, ent->s.angles);
        speed = VectorLength(ent->velocity);
        VectorScale(targetdir, speed, ent->velocity);

	    if (target != ent->homing_target)
        {
	        gi.sound (target, CHAN_AUTO, gi.soundindex("world/klaxon2.wav"), 1, ATTN_NORM, 0);
    	    ent->homing_target = target;
        }
	}

    ent->nextthink = level.time + .1;
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
	rocket->s.modelindex = gi.modelindex (ROCKET_MODEL);
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = ROCKET_FLY_SOUND;
	rocket->classname = "rocket";
    VectorSet(rocket->mins, -10, -3, 0);
    VectorSet(rocket->maxs, 10, 3, 6);
    rocket->mass = 10;
    rocket->health = 10;
    rocket->die = Rocket_Die;
    rocket->takedamage = DAMAGE_YES;
    rocket->monsterinfo.aiflags = AI_NOSTEP;

	if (self->client)
	{
		if (self->client->rocket_toggle == 1)
		{
			rocket->nextthink = level.time + .1;
			rocket->think = guide_think;
			rocket->classname = "guided";
		}
		else if (self->client->rocket_toggle == 2)
		{
			rocket->homing_target = NULL;
			rocket->nextthink = level.time + .1;
			rocket->think = homing_think;
			rocket->classname = "homing";
		}
	}

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);

	if (self->client)
	{
		if (self->client->rocket_toggle == 1)
		{
			self->client->rocketview = true;
			self->client->missile = rocket;
			self->client->was_guided = 1;
			self->client->ps.gunindex = 0;
		}
	}
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
	G_Spawn_Trails(TE_RAILTRAIL, start, tr.endpos, self->s.origin);
	if (water)
	{
		G_Spawn_Trails(TE_RAILTRAIL, start, tr.endpos, tr.endpos);
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

			G_Spawn_Explosion(TE_BFG_EXPLOSION, ent->s.origin, ent->s.origin);
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

	gi.sound (self, CHAN_VOICE, BFG_EXPLODE_SOUND, 1, ATTN_NORM, 0);
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

	G_Spawn_Explosion(TE_BFG_BIGEXPLOSION, self->s.origin, self->s.origin);
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

	if (CVAR_DEATHMATCH)
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

		if (ENT_IS_NOT_MONSTER && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

//PSY: CTF
		//don't target players in CTF
		if (ctf->value && ent->client &&
			self->owner->client &&
			ent->client->resp.ctf_team == self->owner->client->resp.ctf_team
			&& CVAR_DEATHMATCH)
			continue;
//PSY: CTF

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
				G_Spawn_Splash(TE_LASER_SPARKS, 4, self->s.skinnum, tr.endpos, tr.plane.normal, tr.endpos);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		G_Spawn_Trails(TE_BFG_LASER, self->s.origin, tr.endpos, self->s.origin);
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
	bfg->s.sound = BFG_LAUNCH_SOUND;

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}

//============================================================================

/*
void flare_think( edict_t *self )

Purpose: The think function of a flare round.  It generates sparks
         on the flare using a temp entity, and kills itself after
         self->timestamp runs out.
*/
void flare_think(edict_t *self)
{
	// self->timestamp is 30 seconds after the flare was spawned.
    //
    if( level.time > self->timestamp )
    {
	    G_FreeEdict( self );
    }
    // We're still active, so lets see if we need to glow
    //
    else
    {
    //If we're in water, we won't glow.  Otherwise we will...
    //
	    if( !(gi.pointcontents(self->s.origin) & MASK_WATER) )
        {
	        // We're not in water, so proceed as normal
            //
            if (!(self->s.effects & EF_BFG))
				self->s.effects |= EF_BFG;
        }
        else
        {
            // We're in water, so let's not glow anymore.
            //
            self->s.effects &= ~EF_BFG;
        }
	}
    // We'll think again in .2 seconds
    //
    self->nextthink = level.time+0.2;
}

void flare_touch( edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	// Flares don't weigh that much, so let's have them stop
    // the instant they whack into anything.
    //
    VectorClear( ent->velocity );
}

void fire_flaregun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
    edict_t *flare;
    vec3_t dir;
    vec3_t forward, right, up;

    vectoangles (aimdir, dir);
    AngleVectors (dir, forward, right, up);

    flare = G_Spawn();
    VectorCopy (start, flare->s.origin);
    VectorScale (aimdir, speed, flare->velocity);
    VectorSet (flare->avelocity, 300, 300, 300);
    flare->movetype = MOVETYPE_BOUNCE;
    flare->clipmask = MASK_SHOT;
    flare->solid = SOLID_BBOX;
    flare->s.effects |= EF_BFG; //make it glow
    VectorClear (flare->mins);
    VectorClear (flare->maxs);
    flare->s.modelindex = gi.modelindex ("models/objects/flare/tris.md2");
    flare->owner = self;
    flare->touch = flare_touch;
    flare->nextthink = FRAMETIME;
    flare->think = flare_think;
    flare->radius_dmg = damage;
    flare->dmg_radius = damage_radius;
    flare->classname = "flare";
    flare->timestamp = level.time + 30.0; //live for 30 seconds
    gi.linkentity (flare);
}

/*
===========================
Concussion Grenades
===========================
*/
void Concussion_Explode (edict_t *ent)
{
	vec3_t      offset,v;
    edict_t *target;
    float Distance, DrunkTimeAdd;

    // Move it off the ground so people are sure to see it
    VectorSet(offset, 0, 0, 10);
    VectorAdd(ent->s.origin, offset, ent->s.origin);

    if (ent->owner->client)
	    PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

    target = NULL;
    while ((target = findradius(target, ent->s.origin, 520)) != NULL)
    {
	    if (!target->client)
	        continue;       // It's not a player
        if (!visible(ent, target))
            continue;       // The grenade can't see it
        // Find distance
        VectorSubtract(ent->s.origin, target->s.origin, v);
        Distance = VectorLength(v);
        // Calculate drunk factor
        if(Distance < 520/10)
	        DrunkTimeAdd = 20; //completely drunk
        else
            DrunkTimeAdd = 1.5 * 20 * ( 1 / ( ( Distance - 520*2 ) / (520*2) - 2 ) + 1 ); //partially drunk
        if ( DrunkTimeAdd < 0 )
            DrunkTimeAdd = 0; // Do not make drunk at all.

        //Increment the drunk time
        if(target->DrunkTime < level.time)
            target->DrunkTime = DrunkTimeAdd+level.time;
        else
            target->DrunkTime += DrunkTimeAdd;
	}

    // Blow up the grenade
    BecomeExplosion1(ent);
}

void fire_concussiongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
    edict_t *grenade;
    vec3_t  dir;
    vec3_t  forward, right, up;

    vectoangles (aimdir, dir);
    AngleVectors (dir, forward, right, up);

    grenade = G_Spawn();
    VectorCopy (start, grenade->s.origin);
    VectorScale (aimdir, speed, grenade->velocity);
    VectorMA (grenade->velocity, 200 + crandom() * 30.0, up, grenade->velocity);
    VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
    VectorSet (grenade->avelocity, 300, 300, 300);
    grenade->movetype = MOVETYPE_BOUNCE;
    grenade->clipmask = MASK_SHOT;
    grenade->solid = SOLID_BBOX;
    grenade->s.effects |= EF_GRENADE;
    VectorClear (grenade->mins);
    VectorClear (grenade->maxs);
    grenade->s.modelindex = gi.modelindex (HGRENADE_MODEL);
    grenade->owner = self;
    grenade->touch = Grenade_Touch; //Stuff for cluster grenades when they explode
    grenade->nextthink = level.time + timer;
    grenade->think = Concussion_Explode; //stuff for cluster grenades exploding
    grenade->dmg = damage;
    grenade->dmg_radius = damage_radius;
    grenade->classname = "concussion";
    VectorSet(grenade->mins, -3, -3, 0);
    VectorSet(grenade->maxs, 3, 3, 6);
    grenade->mass = 2;
    grenade->health = 10;
    grenade->die = Grenade_Die;
    grenade->takedamage = DAMAGE_YES;
    grenade->monsterinfo.aiflags = AI_NOSTEP;

    gi.linkentity (grenade);
}
