/*==========================================================================
//  x_fbomb.c -- by Patrick Martin              Last updated:  4-10-1998
//--------------------------------------------------------------------------
//  This file contains copies of functions found in 'g_weapon.c'.
//  The copied functions are slightly modified so that they can
//  produce flaming clouds upon exploding.
//========================================================================*/

#include "g_local.h"
#include "w_fbomb.h"
#include "w_fire.h"

void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);

/*==========================/  Flame Grenades  /==========================*/

/*----------------------------------------------------------- New Code -----
//  This is a near-copy of 'grenade_explode'.  In addition to normal
//  damage, the explosion produces a fiery cloud that rains down fire.
//------------------------------------------------------------------------*/
static void FireGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
        vec3_t          cloud = {5, 120, 80};
        vec3_t          timer = {0, 0.5, 0};
        vec3_t          damage = {6, 9, 50};
	vec3_t		radius_damage = {6, 4, 25};
        int             mod;

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

		/* Check for quad damage. */
		if (ent->owner->client->quad_framenum > level.framenum)
		{
			VectorScale(damage, 4, damage);
			VectorScale(radius_damage, 4, radius_damage);
		}
	}

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

	// Flame cloud.
        PBM_FlameCloud(ent->owner, ent->s.origin, cloud, timer, true, damage, radius_damage, 100, 75);

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

/*----------------------------------------------------------- New Code -----
//  This is a near-copy of 'grenade_touch'.  This calls the flame
//  grenade explosion instead of the standard one.
//------------------------------------------------------------------------*/
static void FireGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	FireGrenade_Explode (ent);
}

/*----------------------------------------------------------- New Code -----
//  This is a near-copy of 'fire_grenade'.  This uses the flame
//  grenade explosion and touch functions instead of the standard ones.
//------------------------------------------------------------------------*/
void fire_flamegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
        grenade->movetype     = MOVETYPE_BOUNCE;
        grenade->clipmask     = MASK_SHOT;
        grenade->solid        = SOLID_BBOX;
        grenade->s.effects   |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
        grenade->owner        = self;
        grenade->touch        = FireGrenade_Touch;
        grenade->nextthink    = level.time + timer;
        grenade->think        = FireGrenade_Explode;
        grenade->dmg          = damage;
        grenade->dmg_radius   = damage_radius;
        grenade->classname    = "grenade";

	gi.linkentity (grenade);
}

/*----------------------------------------------------------- New Code -----
//  This is a near-copy of 'fire_grenade2'.  This uses the flame
//  grenade explosion and touch functions instead of the standard ones.
//------------------------------------------------------------------------*/
void fire_flamegrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
        grenade->movetype     = MOVETYPE_BOUNCE;
        grenade->clipmask     = MASK_SHOT;
        grenade->solid        = SOLID_BBOX;
        grenade->s.effects   |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
        grenade->owner        = self;
        grenade->touch        = FireGrenade_Touch;
        grenade->nextthink    = level.time + timer;
        grenade->think        = FireGrenade_Explode;
        grenade->dmg          = damage;
        grenade->dmg_radius   = damage_radius;
        grenade->classname    = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
                FireGrenade_Explode (grenade);
        else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}


/*==========================/  Napalm Rockets  /==========================*/

/*----------------------------------------------------------- New Code -----
//  This is a near-copy of 'rocket_touch'.  In addition to normal
//  damage, the explosion produces a fiery cloud that rains down fire.
//------------------------------------------------------------------------*/
void firerocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
        vec3_t          cloud = {5, 120, 80};
        vec3_t          timer = {0, 0.5, 0};
        vec3_t          damage = {6, 9, 50};
	vec3_t		radius_damage = {6, 4, 25};
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

		/* Check for quad damage. */
		if (ent->owner->client->quad_framenum > level.framenum)
		{
			VectorScale(damage, 4, damage);
			VectorScale(radius_damage, 4, radius_damage);
		}
	}

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

	// Flame cloud.
        PBM_FlameCloud(ent->owner, ent->s.origin, cloud, timer, true, damage, radius_damage, 100, 75);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

/*----------------------------------------------------------- New Code -----
//  This is a near-copy of 'fire_rocket'.  This uses the flame
//  rocket touch functions instead of the standard one.
//------------------------------------------------------------------------*/
void fire_flamerocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
        rocket->movetype     = MOVETYPE_FLYMISSILE;
        rocket->clipmask     = MASK_SHOT;
        rocket->solid        = SOLID_BBOX;
        rocket->s.effects   |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
        rocket->owner        = self;
        rocket->touch        = firerocket_touch;
        rocket->nextthink    = level.time + 8000/speed;
        rocket->think        = G_FreeEdict;
        rocket->dmg          = damage;
        rocket->radius_dmg   = radius_damage;
        rocket->dmg_radius   = damage_radius;
        rocket->s.sound      = gi.soundindex ("weapons/rockfly.wav");
        rocket->classname    = "rocket";

	if (self->client)
                check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*===========================/  END OF FILE  /===========================*/
