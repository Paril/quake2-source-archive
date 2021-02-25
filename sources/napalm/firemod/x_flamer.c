/*==========================================================================
//  x_flamer.c -- by Patrick Martin             Last updated:  2-27-1999
//--------------------------------------------------------------------------
//  This file contains code that produces fire that burns entities.
//  Fire inflicts damage to entities near it.  In addition, there is a
//  chance that an affected entity will catch on fire.  If this happens,
//  the fire will burn the target for some time and inflict more damage.
//  Fire will be extinguished if the burning entity enters at least
//  waist-deep into liquid.  Fire can also spread from entity to entity.
//  However, all frags (and anger) are given only to the entity that
//  originally spawned the fire.
//
//  NOTES:  The following edicts fields defined in 'g_local.h'
//  are used differently here than in other files:
//
//  * count = Chance that a flame will be created and dropped
//      from an exploding fireball.
//  * dmg = Chance that a dropped flame will explode on impact.
//  * light_level = MeansOfDeath for splash damage kills.
//  * pos1 = Fire damage vector for direct hits.  {die, base, chance}.
//      Damage is 1d(die) + base.  Target has chance% of igniting.
//  * pos2 = Same as pos1 except this is used for splash damage.
//  * style = MeansOfDeath for direct hit kills.
//  * timestamp = Time when burning flames will damage nearby entities.
//  * wait = Base duration of flames dropped by a firestorm.
//
//  FILE 4 -- Specialty fire-based attacks.
//
//========================================================================*/

#include "g_local.h"
#include "x_fire.h"
#include "x_flamer.h"


/*==========================/  Flame Grenades  /==========================*/

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'grenade_explode'.  In addition to normal
//  damage, the explosion produces a fiery cloud that rains down fire.
//------------------------------------------------------------------------*/
static void PBM_FireGrenadeExplode (edict_t *ent)
{
	vec3_t		origin;
        vec3_t          cloud = {5, 120, 80};
        vec3_t          timer = {0, 0.5, 1};
        int             mod;

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
        T_RadiusDamage (ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	// Flame cloud.
        PBM_FlameCloud (ent->owner, ent->s.origin, cloud, timer, ent->pos1, ent->pos2, 100, 75);

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

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'grenade_touch'.  This calls the flame
//  grenade explosion instead of the standard one.
//------------------------------------------------------------------------*/
static void PBM_FireGrenadeTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
        PBM_FireGrenadeExplode (ent);
}

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'fire_grenade'.  This uses the flame
//  grenade explosion and touch functions instead of the standard ones.
//------------------------------------------------------------------------*/
void PBM_FireFlameGrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, vec3_t fire_damage, vec3_t splash_damage)
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
        grenade->touch        = PBM_FireGrenadeTouch;
        grenade->nextthink    = level.time + timer;
        grenade->think        = PBM_FireGrenadeExplode;
        grenade->dmg          = damage;
        grenade->dmg_radius   = damage_radius;
        grenade->classname    = "grenade";

        VectorCopy (fire_damage, grenade->pos1);
        VectorCopy (splash_damage, grenade->pos2);

	gi.linkentity (grenade);
}

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'fire_grenade2'.  This uses the flame
//  grenade explosion and touch functions instead of the standard ones.
//------------------------------------------------------------------------*/
void PBM_FireFlameHandGrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held, vec3_t fire_damage, vec3_t splash_damage)
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
        grenade->touch        = PBM_FireGrenadeTouch;
        grenade->nextthink    = level.time + timer;
        grenade->think        = PBM_FireGrenadeExplode;
        grenade->dmg          = damage;
        grenade->dmg_radius   = damage_radius;
        grenade->classname    = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

        VectorCopy (fire_damage, grenade->pos1);
        VectorCopy (splash_damage, grenade->pos2);

	if (timer <= 0.0)
                PBM_FireGrenadeExplode (grenade);
        else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}


/*==========================/  Napalm Rockets  /==========================*/

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'rocket_touch'.  In addition to normal
//  damage, the explosion produces a fiery cloud that rains down fire.
//------------------------------------------------------------------------*/
void PBM_FireRocketTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
        vec3_t          cloud = {5, 120, 80};
        vec3_t          timer = {0, 0.5, 1};
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

        T_RadiusDamage (ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	// Flame cloud.
        PBM_FlameCloud (ent->owner, ent->s.origin, cloud, timer, ent->pos1, ent->pos2, 100, 75);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'fire_rocket'.  This uses the flame
//  rocket touch functions instead of the standard one.
//------------------------------------------------------------------------*/
void PBM_FireFlameRocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage, vec3_t fire_damage, vec3_t splash_damage)
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
        rocket->touch        = PBM_FireRocketTouch;
        rocket->nextthink    = level.time + 8000/speed;
        rocket->think        = G_FreeEdict;
        rocket->dmg          = damage;
        rocket->radius_dmg   = radius_damage;
        rocket->dmg_radius   = damage_radius;
        rocket->s.sound      = gi.soundindex ("weapons/rockfly.wav");
        rocket->classname    = "rocket";

        VectorCopy (fire_damage, rocket->pos1);
        VectorCopy (splash_damage, rocket->pos2);

	if (self->client)
                PBM_CheckFireDodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


/*==========================/  Magic Meteors  /==========================*/

/*------------------------------------------------------/ New Code /--------
//  This damages entities within a certain radius.  There is
//  also a chance that affected entities will catch on fire.
//
//  NOTE:  Damage = 1d(die) + base.  Chance is expressed as percentange.
//------------------------------------------------------------------------*/
void PBM_MeteorRadius
(edict_t *fire, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

        while ((ent = findradius(ent, fire->s.origin, radius)) != NULL)
	{
                if (!CanDamage (ent, fire))  continue;
                if (PBM_Inflammable (ent))  continue;
                if (ent == ignore)  continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
                VectorSubtract (fire->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);
		if (ent == attacker)
			points = points * 0.5;

                VectorSubtract (ent->s.origin, fire->s.origin, dir);
                VectorNormalize2 (dir, v);
                VectorMA (ent->s.origin, -4, v, v);

		if (points > 0)
		{
                        T_Damage (ent, fire, attacker, dir, v, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
		}
	}
}

/*------------------------------------------------------/ New Code /--------
//  The large fireball impacts.  A firestorm is sure to follow...
//------------------------------------------------------------------------*/
void PBM_MeteorTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t  origin;

	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

        if (PBM_InWater (self))
        {       PBM_BecomeSteam (self);
		return;
	}

        if (self->master->client)
                PlayerNoise (self->master, self->s.origin, PNOISE_IMPACT);

/* Calculate position for the explosion entity. */
        VectorNormalize2 (self->velocity, origin);
        VectorMA (self->s.origin, -8, origin, origin);

	if (other->takedamage)
        {
		if (other->flags & FL_IMMUNE_LAVA)
		{	vec3_t	reduced = {0, 0, 0};
                        reduced[1] = self->movedir[1];
                        PBM_BurnDamage (other, self, self->s.origin, plane->normal, reduced, 0, self->style);
		}
		else
                        PBM_BurnDamage (other, self, self->s.origin, plane->normal, self->movedir, 0, self->style);
        }

        PBM_MeteorRadius (self, self->master, self->radius_dmg, other, self->dmg_radius, self->light_level);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_ROCKET_EXPLOSION);
        gi.WritePosition (origin);
        gi.multicast (self->s.origin, MULTICAST_PHS);

        PBM_CreateFireStorm (self->master, self->s.origin, self->pos1, self->pos2, self->move_angles);

        G_FreeEdict (self);
}

/*------------------------------------------------------/ New Code /--------
//  This checks when to put out the fire.  The fire is put out when
//  the burning entity finds water or when the fire is done burning.
//  This is called on every frame.  Used for firestorm meteor only.
//------------------------------------------------------------------------*/
void PBM_CheckMeteor (edict_t *self)
{
/* Put out the fire if it hits water. */
        if (PBM_InWater (self))
        {       PBM_BecomeSteam (self);
		return;
	}

/* The fire burns itself out after a period of time. */
	if (self->burnout < level.time)
        {       PBM_BecomeSmoke (self);
		return;
	}

/* Animate the fire. */
        if (++self->s.frame > FRAME_FIRE_fireb12)
                self->s.frame = FRAME_FIRE_fireb1;

/* Fire is still burning, check again. */
	self->nextthink  = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  Create and launch a big fireball.
//------------------------------------------------------------------------*/
void PBM_FireMeteor
(edict_t *self, vec3_t start, vec3_t dir, int speed, float damage_radius, vec3_t direct_damage, vec3_t rain_damage, vec3_t splash_damage, vec3_t storm)
{
	edict_t  *fireball;

/* Create the fireball. */
	fireball = G_Spawn();
        fireball->s.modelindex    = MD2_FIRE;
        fireball->s.frame         = FRAME_FIRE_igniteb1;
        fireball->s.skinnum       = SKIN_FIRE_skin;
	VectorClear (fireball->mins);
	VectorClear (fireball->maxs);
	VectorCopy (start, fireball->s.origin);
	vectoangles (dir, fireball->s.angles);
	VectorScale (dir, speed, fireball->velocity);
        fireball->s.angles[PITCH] -= 90;
        fireball->movetype        = MOVETYPE_FLYMISSILE;
        fireball->clipmask        = MASK_SHOT;
        fireball->solid           = SOLID_BBOX;
        fireball->takedamage      = DAMAGE_NO;
        fireball->s.effects       = EF_ROCKET;
        fireball->s.renderfx      = RF_FULLBRIGHT;
        fireball->owner           = self;
        fireball->master          = self;
        fireball->classname       = "fire";
        fireball->touch           = PBM_MeteorTouch;
        fireball->burnout         = level.time + 8000/speed;
        fireball->timestamp       = 0;
        fireball->nextthink       = level.time + FRAMETIME;
        fireball->think           = PBM_CheckMeteor;
        VectorCopy (rain_damage, fireball->pos1);
        VectorCopy (splash_damage, fireball->pos2);
        VectorCopy (direct_damage, fireball->movedir);  // Meteor damage.
        VectorCopy (storm, fireball->move_angles);  // Storm dim.
        fireball->dmg_radius      = damage_radius;
        fireball->radius_dmg      = (int)(direct_damage[0] + direct_damage[1]);
        fireball->style           = MOD_METEOR;
        fireball->light_level     = MOD_METEOR_BLAST;
	gi.linkentity (fireball);

/* If fireball is spawned in liquid, it dissolves. */
        if (PBM_InWater (fireball))
        {       PBM_BecomeSteam (fireball);
		return;
	}

/* Check for monster dodge routine. */
	if (self->client)
                PBM_CheckFireDodge (self, fireball->s.origin, dir, speed);
}


/*==========================/  Buster Cannon  /==========================*/

/*----------------------------------------------------/ New Code /--------//
//  This shoots a beam that causes a BIG explosion at the endpoint.
//------------------------------------------------------------------------*/
void PBM_FireBuster
(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int radius_damage, vec3_t radii, vec3_t fire_damage)
{
        vec3_t  end;
        trace_t tr;
        vec3_t  cloud = {10, 100, 100};
        vec3_t  timer = {0.2, 0.4, 1};

	VectorMA (start, 8192, aimdir, end);
        tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);

        if ((tr.fraction < 1) && (tr.surface) && (!(tr.surface->flags & SURF_SKY)))
        {
                if (tr.ent->takedamage)
                {
                        if (tr.ent->client)
                        {       /* Blind target. */
                                tr.ent->client->flash_alpha = 1;
                                tr.ent->client->flash_fade = 0.05;
                        }
                        PBM_Ignite (tr.ent, self, tr.endpos);
                        T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, damage, DAMAGE_ENERGY, MOD_BUSTER);
                }

                VectorMA (tr.endpos, -4, aimdir, end);
                PBM_FlashExplosion (self, tr.endpos, end, radius_damage, radii, NULL, MOD_FLASH);
                PBM_FlameCloud (self, tr.endpos, cloud, timer, fire_damage, fire_damage, 0, 100);
	}

	if (self->client)
                PlayerNoise (self, tr.endpos, PNOISE_IMPACT);
}


/*===========================/  END OF FILE  /===========================*/
