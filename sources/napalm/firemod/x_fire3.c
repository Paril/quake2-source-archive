/*==========================================================================
//  x_fire3.c -- by Patrick Martin              Last updated:  2-7-1999
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
//  FILE 3 -- Generic fire-based attacks.
//
//========================================================================*/

#include "g_local.h"
#include "x_fire.h"


/*============================/  Fire Dodge  /============================*/

/*-------------------------------------------------------- Mirror Code -----
//  This is a copy of 'check_dodge' found in 'g_weapons.c'.
//  Because that function is static, and I do not want to
//  modify that file, a copy is placed here.
//------------------------------------------------------------------------*/
void PBM_CheckFireDodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	/* easy mode only ducks one quarter the time */
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


/*==========================/  Vector Spread  /==========================*/

/*------------------------------------------------------/ New Code /--------
//  This adjusts a vector's angles with the spread given.
//
//  NOTE:  'spread' vector fields --
//         [0] = PITCH = Vertical spread.
//         [1] = YAW   = Horizontal spread.
//         [2] = ROLL  = No effect (should always be 0).
//------------------------------------------------------------------------*/
void PBM_FireAngleSpread (vec3_t spread, vec3_t dir)
{
        vec3_t  v;

        vectoangles (dir, v);

        v[PITCH] += (random() - 0.5) * spread[PITCH];
        v[YAW]   += (random() - 0.5) * spread[YAW];
        v[ROLL]  += (random() - 0.5) * spread[ROLL];

        AngleVectors (v, dir, NULL, NULL);
}

/*------------------------------------------------------/ New Code /--------
//  This converts horizontal/vertical units into degrees.
//------------------------------------------------------------------------*/
void PBM_SpreadConverter (vec3_t spread, float hspread, float vspread)
{
        float   two_degrees = 360/M_PI;

        spread[0] = (atan(hspread/8192) * two_degrees);
        spread[1] = (atan(vspread/8192) * two_degrees);
        spread[2] = 0;
}


/*======================/  Generic Flame Attacks  /======================*/

/*------------------------------------------------------/ New Code /--------
//  The fireball impacts and delivers pain and flames.
//------------------------------------------------------------------------*/
void PBM_FireballTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

        if (PBM_InWater (self))
        {       PBM_BecomeSteam (self);
		return;
	}

	if (self->owner->client)
                PlayerNoise (self->owner, self->s.origin, PNOISE_IMPACT);

/* Burn! */
        PBM_BurnRadius (self, self->dmg_radius, self->pos2, other, self->light_level);

/* Calculate position for the explosion entity. */
        VectorNormalize (self->velocity);
        VectorMA (self->s.origin, -8, self->velocity, self->s.origin);
        gi.linkentity (self);

	if (other->takedamage)
	{
		if (other->flags & FL_IMMUNE_LAVA)
		{	vec3_t	reduced = {0, 0, 0};
			reduced[1] = self->pos1[1];
                        PBM_BurnDamage (other, self, self->s.origin, plane->normal, reduced, 0, self->style);
		}
		else
                        PBM_BurnDamage (other, self, self->s.origin, plane->normal, self->pos1, 0, self->style);
	}
	else
        {       if ((rand() % 100) < self->count)
                        PBM_EasyFireDrop (self);
	}

        PBM_BecomeSmallExplosion (self);
}

/*------------------------------------------------------/ New Code /--------
//  Create and launch a fireball.
//------------------------------------------------------------------------*/
void PBM_FireFlamer
(edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance, int mod_strike, int mod_splash)
{
	edict_t  *fireball;

/* Adjust fireball direction with 'spread'. */
        PBM_FireAngleSpread (spread, dir);

/* Create the fireball. */
	fireball = G_Spawn();
        fireball->s.modelindex    = MD2_FIRE;
        fireball->s.frame         = FRAME_FIRE_ignite1;
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
        fireball->svflags         = SVF_DEADMONSTER;   // Hyperblaster fix...
        fireball->s.effects       = 0;
        fireball->s.renderfx      = RF_FULLBRIGHT;
        fireball->owner           = self;
        fireball->master          = self;
        fireball->classname       = "fire";
        fireball->touch           = PBM_FireballTouch;
        fireball->burnout         = level.time + 8000/speed;
        fireball->timestamp       = 0;
        fireball->nextthink       = level.time + FRAMETIME;
        fireball->think           = PBM_CheckFire;
	VectorCopy(damage, fireball->pos1);
	VectorCopy(radius_damage, fireball->pos2);
        fireball->dmg_radius      = RADIUS_FIRE_SPLASH;
        fireball->count           = rain_chance;
        fireball->dmg             = blast_chance;
        fireball->style           = mod_strike;
        fireball->light_level     = mod_splash;
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

/*----------------------------------------------------/ New Code /--------//
//  This spawns a small, fiery explosion instantly.  This is
//  the instant strike version of the above fireball attack.
//------------------------------------------------------------------------*/
void PBM_FireInstantFlamer
(edict_t *attacker, vec3_t start, vec3_t spread, vec3_t aimdir, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance, int mod_strike, int mod_splash)
{
        edict_t *fire;
        vec3_t  end;
        vec3_t  dir;
        trace_t tr;

/* Is the source underwater?  If so, make an ent that produces steam. */
        if (gi.pointcontents (start) & MASK_WATER)
        {
                fire = G_Spawn();
                fire->s.modelindex = gi.modelindex("");
                VectorClear (fire->mins);
                VectorClear (fire->maxs);
                VectorCopy (start, fire->s.origin);
                fire->movetype   = MOVETYPE_NONE;
                fire->solid      = SOLID_NOT;
                fire->takedamage = DAMAGE_NO;
                fire->clipmask   = 0;
                fire->touch      = NULL;
                gi.linkentity (fire);

                PBM_BecomeSteam (fire);
                return;
        }

/* Change aiming direction. */
        VectorCopy (aimdir, dir);
        PBM_FireAngleSpread (spread, dir);

        VectorMA (start, 8192, dir, end);
        tr = gi.trace (start, NULL, NULL, end, attacker, MASK_SHOT | MASK_WATER);

/* Burn the target and other targets near it. */
        if (tr.fraction >= 1)
                return;

        if (!tr.surface || (tr.surface->flags & SURF_SKY))
                return;

        VectorCopy (tr.endpos, end);

        fire = G_Spawn();
        fire->s.modelindex = gi.modelindex("");
        VectorClear (fire->mins);
        VectorClear (fire->maxs);
        VectorCopy (end, fire->s.origin);
        fire->movetype   = MOVETYPE_NONE;
        fire->solid      = SOLID_NOT;
        fire->takedamage = DAMAGE_NO;
        fire->clipmask   = 0;
        fire->owner      = attacker;
        fire->master     = attacker;
        fire->enemy      = NULL;
        fire->classname  = "fire";
        fire->touch      = NULL;
        VectorCopy (damage, fire->pos1);
        VectorCopy (radius_damage, fire->pos2);
        fire->dmg_radius = RADIUS_FIRE_SPLASH;
        fire->count      = rain_chance;
        fire->dmg        = blast_chance;
        fire->style       = mod_strike;
        fire->light_level = mod_splash;
        gi.linkentity (fire);

        if (tr.contents & MASK_WATER)
        {       PBM_BecomeSteam (fire);
                return;
        }

        PBM_BurnRadius (fire, fire->dmg_radius, fire->pos2, tr.ent, mod_splash);

        VectorMA (fire->s.origin, -8, dir, fire->s.origin);
        gi.linkentity (fire);

        if (tr.ent->takedamage)
        {
                if (tr.ent->flags & FL_IMMUNE_LAVA)
                {       vec3_t  reduced = {0, 0, 0};
                        reduced[1] = fire->pos1[1];
                        PBM_BurnDamage (tr.ent, fire, fire->s.origin, tr.plane.normal, reduced, 0, mod_strike);
                }
                else
                        PBM_BurnDamage (tr.ent, fire, fire->s.origin, tr.plane.normal, fire->pos1, 0, mod_strike);
        }
        else
        {       if ((rand() % 100) < fire->count)
                        PBM_EasyFireDrop (fire);
        }

        PBM_BecomeSmallExplosion (fire);

        if (attacker->client)
                PlayerNoise (attacker, tr.endpos, PNOISE_IMPACT);
}

/*----------------------------------------------------/ New Code /--------//
//  This fires multiple flame strikes simultaneously.
//------------------------------------------------------------------------*/
void PBM_FireExplosiveBurst (edict_t *self, vec3_t start, vec3_t spread, vec3_t aimdir, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance, int count, int mod_strike, int mod_splash)
{
        int     i;

	for (i = 0; i < count; i++)
                PBM_FireInstantFlamer (self, start, spread, aimdir, damage, radius_damage, rain_chance, blast_chance, mod_strike, mod_splash);
}


/*===========================/  Flamethrower  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  The fat fireball impacts and burns stuff up.
//------------------------------------------------------------------------*/
void PBM_FlameThrowerTouch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
        {       G_FreeEdict (self);
		return;
	}

        if (PBM_InWater (self))
        {       PBM_BecomeSteam (self);
		return;
	}

/* Flamethrower shots do not "impact". */

/* Burn! */
        PBM_BurnRadius (self, self->dmg_radius, self->pos2, other, self->light_level);

/* Calculate position for the explosion entity. */
        VectorNormalize (self->velocity);
        VectorMA (self->s.origin, -8, self->velocity, self->s.origin);
        gi.linkentity (self);

	if (other->takedamage)
	{
		if (other->flags & FL_IMMUNE_LAVA)
		{	vec3_t	reduced = {0, 0, 0};
			reduced[1] = self->pos1[1];
                        PBM_BurnDamage (other, self, self->s.origin, plane->normal, reduced, 0, self->style);
		}
		else
                        PBM_BurnDamage (other, self, self->s.origin, plane->normal, self->pos1, 0, self->style);
	}
	else
        {       if ((rand() % 100) < self->count)
                        PBM_EasyFireDrop (self);
	}

/* Stop the flame. */
	self->touch        = NULL;
        self->solid        = SOLID_NOT;
        self->takedamage   = DAMAGE_NO;
        self->clipmask     = 0;
	self->movetype     = MOVETYPE_NONE;
        VectorClear (self->velocity);
        gi.linkentity (self);
}

/*------------------------------------------------------/ New Code /--------
//  This animates the fireball/explosion.  The magic numbers in this
//  function represent the frame numbers in id's r_explode md2.
//  While thinking, the shot will burn nearby entities, except its
//  owner; thus making the shot an area effect weapon.
//------------------------------------------------------------------------*/
void PBM_FlameThrowerThink (edict_t *self)
{
        if (++self->s.frame > 6)
	{	G_FreeEdict (self);
		return;
	}

/* Put out the fire if it hits water. */
	if (gi.pointcontents(self->s.origin) & MASK_WATER)
        {       PBM_BecomeSteam (self);
		return;
	}

        PBM_BurnRadius (self, self->dmg_radius, self->pos2, self->owner, self->light_level);

	self->s.skinnum++;
	self->nextthink = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  Create and launch a fat fireball.
//------------------------------------------------------------------------*/
void PBM_FireFlameThrower
(edict_t *self, vec3_t start, vec3_t spread, vec3_t dir, int speed, vec3_t damage, vec3_t radius_damage, int rain_chance, int blast_chance, int mod_strike, int mod_splash)
{
	edict_t  *fireball;

/* Adjust fireball direction with 'spread'. */
        PBM_FireAngleSpread (spread, dir);

/* Create the fireball. */
	fireball = G_Spawn();
        fireball->s.modelindex = gi.modelindex("models/objects/r_explode/tris.md2");
        fireball->s.frame      = 2;
        fireball->s.skinnum    = 1;
	VectorClear (fireball->mins);
	VectorClear (fireball->maxs);
	VectorCopy (start, fireball->s.origin);
	vectoangles (dir, fireball->s.angles);
	VectorScale (dir, speed, fireball->velocity);
        fireball->s.angles[PITCH] -= 90;
        fireball->s.angles[YAW]   += rand() % 360;
        fireball->movetype        = MOVETYPE_FLYMISSILE;
        fireball->clipmask        = MASK_SHOT;
        fireball->solid           = SOLID_BBOX;
        fireball->takedamage      = DAMAGE_NO;
        fireball->svflags         = SVF_DEADMONSTER;   // Hyperblaster fix...
        fireball->s.effects       = 0;
        fireball->s.renderfx      = RF_FULLBRIGHT;
        fireball->owner           = self;
        fireball->master          = self;
        fireball->classname       = "fire";
        fireball->touch           = PBM_FlameThrowerTouch;
        fireball->nextthink       = level.time + FRAMETIME * 2;
        fireball->think           = PBM_FlameThrowerThink;
	VectorCopy(damage, fireball->pos1);
	VectorCopy(radius_damage, fireball->pos2);
        fireball->dmg_radius      = RADIUS_FIRE_SPLASH;
        fireball->count           = rain_chance;
        fireball->dmg             = blast_chance;
        fireball->style           = mod_strike;
        fireball->light_level     = mod_splash;
	gi.linkentity (fireball);

/* If fireball is spawned in liquid, it dissolves. */
        if (PBM_InWater (fireball))
        {       PBM_BecomeSteam (fireball);
		return;
	}

/* Flamethrower is an area effect weapon, so don't bother dodging. */
}


/*========================/  Incendiary Rounds  /========================*/

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'fire_lead'.  In addition to normal
//  pellet effects, this can also set targets on fire.
//------------------------------------------------------------------------*/
static void PBM_FireIncendiaryRound
(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int burn_chance, int te_impact, int hspread, int vspread, int mod)
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
                                if ((rand() % 100) < burn_chance)
                                        PBM_Ignite (tr.ent, self, tr.endpos);
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

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'fire_bullet'.  Shoots incendiary rounds
//  instead of normal lead rounds.
//------------------------------------------------------------------------*/
void PBM_FireFlameBullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int burn_chance, int hspread, int vspread, int mod)
{
        PBM_FireIncendiaryRound (self, start, aimdir, damage, kick, burn_chance, TE_SPARKS, hspread, vspread, mod);
}

/*------------------------------------------------------/ New Code /--------
//  This is a near-copy of 'fire_shotgun'.  Shoots incendiary rounds
//  instead of normal lead rounds.
//------------------------------------------------------------------------*/
void PBM_FireFlameShotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int burn_chance, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
                PBM_FireIncendiaryRound (self, start, aimdir, damage, kick, burn_chance, TE_SPARKS, hspread, vspread, mod);
}


/*===========================/  END OF FILE  /===========================*/
