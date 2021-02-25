/*==========================================================================
//  x_fire2.c -- by Patrick Martin              Last updated:  2-27-1999
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
//  FILE 2 -- Pyrotechnics.
//
//========================================================================*/

#include "g_local.h"
#include "x_fire.h"


/*===========================/  Small Flames  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This checks when to put out the fire.  The fire is put out when
//  the burning entity finds water or when the fire is done burning.
//  The fire can (but does not need to) damage/ignite nearby entities.
//  This is called on every frame.  Used for small flames only.
//------------------------------------------------------------------------*/
void PBM_CheckFire (edict_t *self)
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
        if (++self->s.frame > FRAME_FIRE_fire12)
                self->s.frame = FRAME_FIRE_fire1;

/* The fire may inflict a small amount of burn damage. */
	if (self->timestamp)
		if (self->timestamp < level.time)
		{
			vec3_t	damage = {3, 0, 5};

                        PBM_CheckMaster (self);
                        PBM_BurnRadius (self, 50, damage, NULL, MOD_FIRE_SPLASH);
			self->timestamp = level.time + random();
		}

/* Fire is still burning, check again. */
	self->nextthink  = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  Fire that drops onto an entity may set it on fire (if possible)
//  and explode to damage and possibly ignite nearby targets.
//------------------------------------------------------------------------*/
void PBM_FireDropTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
/* Check if floor is the sky.  If so, disappear. */
	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

/* If in water, dissolve into steam. */
        if (PBM_InWater (self))
        {       PBM_BecomeSteam (self);
		return;
	}

/* Dropped flames do NOT create pnoise. */

/* Check if it explodes. */
        if ( (other->solid != SOLID_BSP) || ((rand() % 100) < self->dmg) )
        {
        /* NOTE:  Use MeansOfDeath of parent attack. */
                if (other->takedamage)
                {
                        if (!(other->flags & FL_IMMUNE_LAVA))
                                PBM_BurnDamage (other, self, self->s.origin, plane->normal, self->pos1, 0, self->style);
                }
                PBM_BurnRadius (self, self->dmg_radius, self->pos2, other, self->light_level);

        /* Calculate position for the explosion entity. */
                VectorNormalize (self->velocity);
                VectorMA (self->s.origin, -8, self->velocity, self->s.origin);

                PBM_BecomeSmallExplosion (self);
		return;
	}

/* The fire remains burning. */
	 VectorClear (self->velocity);
	 self->solid = SOLID_NOT;
	 VectorSet (self->mins, -4, -4, 0);
	 VectorSet (self->maxs, 4, 4, 24);
         gi.linkentity (self);
}

/*------------------------------------------------------/ New Code /--------
//  This creates a flame that drops straight down.
//------------------------------------------------------------------------*/
void PBM_FireDrop
(edict_t *attacker, vec3_t spot, vec3_t damage, vec3_t radius_damage, float duration_base, int blast_chance, int mod_strike, int mod_splash)
{
	edict_t *fire;

	fire = G_Spawn();
	fire->s.modelindex = MD2_FIRE;
        fire->s.frame      = FRAME_FIRE_ignite1;
        fire->s.skinnum    = SKIN_FIRE_skin;
	VectorClear (fire->mins);
	VectorClear (fire->maxs);
        VectorCopy (spot, fire->s.origin);
	VectorClear (fire->s.angles);
	VectorClear (fire->velocity);
	fire->movetype     = MOVETYPE_TOSS;
	fire->clipmask     = MASK_SHOT;
	fire->solid        = SOLID_BBOX;
	fire->takedamage   = DAMAGE_NO;
        fire->svflags      = SVF_DEADMONSTER;   // The hyperblaster fix...
	fire->s.effects    = 0;
	fire->s.renderfx   = RF_FULLBRIGHT;
        fire->owner        = fire;
        fire->master       = attacker;
	fire->classname    = "fire";
	fire->touch        = PBM_FireDropTouch;
        fire->burnout      = level.time + duration_base + random() * FLAME_EXTRA_TIME;
	fire->timestamp    = level.time;
	fire->nextthink    = level.time + FRAMETIME;
	fire->think        = PBM_CheckFire;
        VectorCopy (damage, fire->pos1);
        VectorCopy (radius_damage, fire->pos2);
        fire->dmg_radius   = RADIUS_FIRE_SPLASH;
        fire->dmg          = blast_chance;
        fire->style        = mod_strike;
        fire->light_level  = mod_splash;

	gi.linkentity (fire);
}

/*------------------------------------------------------/ New Code /--------
//  This is a support function for 'PBM_FireDrop()'.
//------------------------------------------------------------------------*/
void PBM_EasyFireDrop (edict_t *self)
{
        PBM_FireDrop (self->owner, self->s.origin, self->pos1, self->pos2, FLAME_TIME, self->dmg, self->style, self->light_level);
}


/*===========================/  Flame Clouds  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This causes part of the flame cloud to explode and drop flames.
//------------------------------------------------------------------------*/
void PBM_CloudBurst (edict_t *self)
{
	if ((rand() % 100) < self->count)
                PBM_EasyFireDrop (self);

        PBM_BecomeSmallExplosion (self);
}

/*------------------------------------------------------/ New Code /--------
//  This causes part of the flame cloud to damage as it erupts.
//------------------------------------------------------------------------*/
void PBM_CloudBurstDamage (edict_t *self)
{
        PBM_BurnRadius (self, self->dmg_radius, self->pos2, NULL, MOD_CLOUD_BURST);
        PBM_CloudBurst (self);
}

/*------------------------------------------------------/ New Code /--------
//  This creates a flaming cloud that can rain fire.
//
//  NOTE:  'cloud' vector fields --
//         [0] = Number of mini-explosions.
//         [1] = X and Y dimensions of cloud.
//         [2] = Z dimension of cloud.
//
//         'timer' vector fields --
//         [0] = Delay before any explosion erupts.
//         [1] = Extra random delay after first delay per explosion.
//         [2] = If non-zero, cloud is damaging.
//------------------------------------------------------------------------*/
void PBM_FlameCloud
(edict_t *attacker, vec3_t start, vec3_t cloud, vec3_t timer, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance)
{
        vec3_t  spot;
        edict_t *smoke;
	int  quota = (int)cloud[0];
        int  check = quota * 3;      /* Use correction after this #. */
        int  limit = check + quota;  /* Max. number of attempts. */
	int  tally = 0;           /* Number of flames created. */
	int  trials;              /* Number of attempts. */
        qboolean deadly = false;  /* If true, explosions hurt. */

        if (timer[2])
                deadly = true;

	for (trials = 0; (trials < limit) && (tally < quota); trials++)
	{
        /* Pick any spot within cloud space. */
		VectorCopy (start, spot);
		spot[0] += ((random() - 0.5) * cloud[1]);
		spot[1] += ((random() - 0.5) * cloud[1]);
		spot[2] += ((random() - 0.5) * cloud[2]);

        /* If spot is in a solid, then either the fire is killed or
           placed back into empty space.  Unless the loop is near the
           counter limit, the flame is killed.
        */
                if ((trials >= check) && (gi.pointcontents(spot) & MASK_SOLID))
                {
                        trace_t tr;
                        vec3_t dir;

                        tr = gi.trace (start, NULL, NULL, spot, NULL, MASK_SHOT);

                        VectorCopy (tr.endpos, spot);
                        VectorSubtract (start, spot, dir);
                        VectorNormalize (dir);
                        VectorMA (spot, 10, dir, spot);
                }

        /* Create a fireball-shaped cloud if in empty space. */
                if (!(gi.pointcontents(spot) & MASK_SOLID))
		{
			smoke = G_Spawn();
                        smoke->s.modelindex = gi.modelindex("");
			VectorClear (smoke->mins);
			VectorClear (smoke->maxs);
			VectorCopy (spot, smoke->s.origin);
                        smoke->movetype   = MOVETYPE_NONE;
                        smoke->solid      = SOLID_NOT;
                        smoke->takedamage = DAMAGE_NO;
                        smoke->clipmask   = 0;
                        smoke->owner      = attacker;
                        smoke->master     = attacker;
                        smoke->enemy      = NULL;
                        smoke->classname  = "fire";
                        smoke->touch      = NULL;
                        smoke->nextthink  = level.time + timer[0] + random() * timer[1];
                        if (PBM_InWater (smoke))
                                smoke->think      = PBM_BecomeSteam;
                        else if (deadly)
                                smoke->think      = PBM_CloudBurstDamage;
			else
                                smoke->think      = PBM_CloudBurst;
			VectorCopy (damage, smoke->pos1);
			VectorCopy (radius_damage, smoke->pos2);
                        smoke->dmg_radius = RADIUS_FIRE_SPLASH;
                        smoke->count      = rain_chance;
                        smoke->dmg        = blast_chance;

                        smoke->style        = MOD_CLOUD_FIRE;
                        smoke->light_level  = MOD_CLOUD_FIRE;

			gi.linkentity (smoke);

			tally++;
		}
	}
}


/*============================/  Firestorms  /============================*/

/*----------------------------------------------------/ New Code /--------//
//  This unlinks and removes the firestorm.
//------------------------------------------------------------------------*/
void PBM_RemoveFireStorm (edict_t *self)
{
        if (self->master)
        {
                if (self->master->stormlink == self)
                        self->master->stormlink = self->stormlink;
                else if (self->master->stormlink)
                {
                        edict_t *storm;

                        for (storm = self->master->stormlink; (storm->stormlink && (storm->stormlink != self)); storm = storm->stormlink);
                        if (storm->stormlink == self)
                                storm->stormlink = self->stormlink;
                }
        }

        G_FreeEdict (self);
}

/*----------------------------------------------------/ New Code /--------//
//  This causes the firestorm to drop flames.
//------------------------------------------------------------------------*/
void PBM_FireStormThink (edict_t *self)
{
        vec3_t  spot, spot2;
        trace_t tr;
        float   best_tr = -1;
        int     i;

        if (self->timestamp < level.time)
        {       /* Time's up!  End the firestorm. */
                PBM_RemoveFireStorm (self);
                return;
        }

/* Find a spot for flame to appear. */
        for (i=0; i<4; i++)
        {
                spot[0] = self->s.origin[0] + ((random() - 0.5) * self->dmg_radius);
                spot[1] = self->s.origin[1] + ((random() - 0.5) * self->dmg_radius);
                spot[2] = self->s.origin[2] + ((random() - 0.5) * self->dmg_radius);
                if (!(gi.pointcontents (spot) & MASK_SOLID))
                        break;

                spot[2] = self->s.origin[2];
                if (!(gi.pointcontents (spot) & MASK_SOLID))
                        break;

                /* Spot is in a solid, so find a alternate point. */
                tr = gi.trace (self->s.origin, NULL, NULL, spot, NULL, MASK_SOLID);
                if (tr.fraction > best_tr)
                        VectorCopy (tr.endpos, spot);
        }

/* Raise the spot to the ceiling. */
        VectorCopy (spot, spot2);
        spot2[2] += 8192;
        tr = gi.trace (spot, NULL, NULL, spot2, NULL, MASK_SOLID);

        PBM_FireDrop (self->master, tr.endpos, self->pos1, self->pos2, self->wait, 100, MOD_FIRESTORM, MOD_FIRESTORM);

        self->nextthink = level.time + FRAMETIME;
}

/*----------------------------------------------------/ New Code /--------//
//  This links the storm generator directly to its master as the
//  most recent firestorm created.
//
//  Why do we have links?  The player may have only a finite
//  number of active firestorms.  If the player has more active
//  firestorms than allowed, the oldest firestorms are removed.
//------------------------------------------------------------------------*/
void PBM_LinkFireStorm (edict_t *self)
{
        if (self->master)
        {
                edict_t *storm, *next;
                int     level = 0;
                int     maxstorms = (int)MAX_FIRESTORMS;

                if (PBM_NeedSpeed () || (MAX_FIRESTORMS < 1))
                        maxstorms = 1;

                /* Link the new firestorm. */
                self->stormlink = self->master->stormlink;
                self->master->stormlink = self;

                /* Remove all extra firestorms. */
                for (storm = self->master->stormlink; storm; storm = next)
                {
                        level++;
                        next = storm->stormlink;
                        if (level > maxstorms)
                                G_FreeEdict (storm);
                        else if (level == maxstorms)
                                storm->stormlink = NULL;
                }
        }

/* Start raining death and destruction! */
        self->think = PBM_FireStormThink;
        PBM_FireStormThink (self);
}

/*----------------------------------------------------/ New Code /--------//
//  This creates a storm generator that rains down fire.
//  This is similar to the powered-up Hellstaff in Heretic.
//
//  NOTE:  'dim' vector fields --
//         [0] = Delay in seconds before firestorm start raining.
//         [1] = Duration of firestorm in seconds.
//         [2] = Firestorm radius.
//------------------------------------------------------------------------*/
void PBM_CreateFireStorm
(edict_t *attacker, vec3_t start, vec3_t damage, vec3_t radius_damage, vec3_t dim)
{
        edict_t *storm;
        float   flametime = dim[1];

/* Duration of dropped flames is adjustable. */
        if (flametime > 5)
                flametime = 5;
        if (flametime < FLAME_TIME)
                flametime = FLAME_TIME;

/* Create the rain generator. */
        storm = G_Spawn();
        storm->s.modelindex = gi.modelindex("");
        VectorClear (storm->mins);
        VectorClear (storm->maxs);
        VectorCopy (start, storm->s.origin);
        storm->movetype   = MOVETYPE_NONE;
        storm->solid      = SOLID_NOT;
        storm->takedamage = DAMAGE_NO;
        storm->clipmask   = 0;
        storm->owner      = attacker;
        storm->master     = attacker;
        storm->enemy      = NULL;
        storm->classname  = "firestorm";
        storm->touch      = NULL;
        VectorCopy (damage, storm->pos1);
        VectorCopy (radius_damage, storm->pos2);
        storm->dmg_radius = dim[2];
        storm->wait       = flametime;
        storm->nextthink  = level.time + dim[0];
        storm->timestamp  = storm->nextthink + dim[1];
        storm->think      = PBM_LinkFireStorm;
        storm->stormlink  = NULL;
        gi.linkentity (storm);
}


/*=========================/  Flash Explosion  /=========================*/

/*-------------------------------------------------------- New Code --------
//  This checks if a target can be damaged from a point.
//------------------------------------------------------------------------*/
qboolean PBM_CanDamageFromPoint (edict_t *targ, vec3_t point)
{
	vec3_t	dest;
	trace_t	trace;

/* bmodels need special checking because their origin is 0,0,0 */
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
                trace = gi.trace (point, vec3_origin, vec3_origin, dest, NULL, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}
	
        trace = gi.trace (point, vec3_origin, vec3_origin, targ->s.origin, NULL, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
        trace = gi.trace (point, vec3_origin, vec3_origin, dest, NULL, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
        trace = gi.trace (point, vec3_origin, vec3_origin, dest, NULL, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
        trace = gi.trace (point, vec3_origin, vec3_origin, dest, NULL, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
        trace = gi.trace (point, vec3_origin, vec3_origin, dest, NULL, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;


	return false;
}

/*-------------------------------------------------------- New Code --------
//  This destroys 'ent' if it is a projectile.  'true' is returned
//  if entity is destroyed, 'false' is returned if entity was not
//  affected.
//------------------------------------------------------------------------*/
qboolean PBM_NegateMissile (edict_t *ent)
{
/*
// WARNING:
//    Freeing edicts can be very dangerous, especially if 'ent' has
//    team links (e.g., old 3.05 BFG and its lasers) or if 'ent' is
//    the attacking projectile.  Ideally, projectiles should be checked
//    on a case-by-case basis.  Uncomment this at your own risk!!!

        if (ent->clipmask == MASK_SHOT)
        {
                G_FreeEdict (ent);
                return true;
        }
*/

/* Entity was not affected. */
        return false;
}

/*-------------------------------------------------------- New Code --------
//  This produces the effects of a blinding explosion.  Targets suffer
//  various effects depending how close to ground zero they were.
//
//  NOTE:  'radii' vector fields --
//         [0] = Burn radius.  Targets ignite.
//         [1] = Blast radius.  Targets take damage.
//         [2] = Flash radius.  Vision flashes; monsters may get stunned.
//------------------------------------------------------------------------*/
void PBM_FlashRadius
(edict_t *attacker, vec3_t origin, float damage, vec3_t radii, edict_t *ignore, int mod)
{
	float	points;
	edict_t	*ent = NULL;
        vec3_t  spot;
	vec3_t	dir;
        float   dist;
        float   diameter = radii[1] * 2;

/* Do not allow division by zero below. */
        if (diameter < 1)
                diameter = 1;

/* Find everyone within flash radius and hurt them in some way. */
        while ((ent = findradius(ent, origin, radii[2])) != NULL)
	{
                if (!PBM_CanDamageFromPoint (ent, origin))
                        continue;
                if (ent == ignore)
                        continue;
		if (!ent->takedamage)
                {
                        PBM_NegateMissile (ent);
			continue;
                }

                VectorSubtract (ent->s.origin, origin, dir);
                dist = VectorLength (dir);

        /* If player, blind him or her. */
		if (ent->client)
		{
                        float   fade = dist * 0.0002;

                        if (fade < 0.1)
                                fade = 0.1;
                        if ((fade < ent->client->flash_fade) || (ent->client->flash_fade <= 0))
                                ent->client->flash_fade = fade;
                        ent->client->flash_alpha = 1;
		}

        /* If entity is within the blast, hurt it. */
                if (dist <= radii[1])
		{
                        VectorAdd (ent->mins, ent->maxs, spot);
                        VectorMA (ent->s.origin, 0.5, spot, spot);
                        VectorSubtract (origin, spot, spot);

                        /* Minimum damage is ~30% of full radius damage. */
                        points = damage * (1.0 - sqrt(dist/diameter));
                        if (ent == attacker)
                                points *= 0.5;
                        if (points < 1)
                                points = 1;

                        VectorSubtract (ent->s.origin, origin, dir);
                        VectorNormalize2 (dir, spot);
                        VectorMA (ent->s.origin, -4, spot, spot);
                        T_Damage (ent, attacker, attacker, dir, spot, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);

                        /* If close to ground zero, set it on fire. */
                        if (dist <= radii[0])
                                if (ent && (ent->health > ent->gib_health))
                                {       ent->pain_debounce_time = 0;
                                        PBM_Ignite (ent, attacker, spot);
                                }
		}
                else if (!ent->client && (ent->svflags & SVF_MONSTER))
                {       /* If monster, try to stun it briefly. */
                        if (ent->health > 0)
                                if ( ent->pain && (!(ent->monsterinfo.aiflags & AI_DUCKED)) )
                                {       ent->pain_debounce_time = 0;
                                        ent->pain (ent, ent, 0, 1);
                                }
                }
	}
}

/*----------------------------------------------------/ New Code /--------//
//  This creates the flash explosion's effects.
//------------------------------------------------------------------------*/
void PBM_FlashEffects (vec3_t p1, vec3_t p2)
{
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_PLAIN_EXPLOSION);
        gi.WritePosition (p2);
        gi.multicast (p1, MULTICAST_ALL);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_NUKEBLAST);
        gi.WritePosition (p2);
        gi.multicast (p1, MULTICAST_ALL);
}

/*----------------------------------------------------/ New Code /--------//
//  This creates a flash explosion.  Used for nukes or other weapons
//  of similar destructive power.
//------------------------------------------------------------------------*/
void PBM_FlashExplosion
(edict_t *attacker, vec3_t p1, vec3_t p2, float damage, vec3_t radii, edict_t *ignore, int mod)
{
        PBM_FlashRadius (attacker, p1, damage, radii, ignore, mod);
        PBM_FlashEffects (p1, p2);
}


/*===========================/  END OF FILE  /===========================*/
