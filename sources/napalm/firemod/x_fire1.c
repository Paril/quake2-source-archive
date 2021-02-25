/*==========================================================================
//  x_fire1.c -- by Patrick Martin              Last updated:  2-27-1999
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
//  FILE 1 -- The Core Burn Code.
//
//========================================================================*/

#include "g_local.h"
#include "x_fire.h"


/*========================/  General Fire Code  /========================*/

/*-------------------------------------------------------- New Code --------
//  This precaches all the sounds and models used by this mod.
//------------------------------------------------------------------------*/
void PBM_CacheFire (void)
{
/* Sounds */
        gi.soundindex ("player/burn1.wav");
        gi.soundindex ("player/burn2.wav");
        gi.soundindex ("player/lava1.wav");
        gi.soundindex ("player/lava2.wav");
        gi.soundindex ("weapons/grenlx1a.wav");
        gi.soundindex ("world/steam1.wav");
        gi.soundindex ("world/steam2.wav");

/* Models */
        gi.modelindex ("models/objects/r_explode/tris.md2");
        MD2_FIRE;
}

/*-------------------------------------------------------- New Code --------
//  This removes every fire from all burning entities.  This
//  should always be called whenever a client changes levels.
//
//  NOTE:  This causes the flames to simply disappear.
//------------------------------------------------------------------------*/
void PBM_KillAllFires (void)
{
        edict_t *ent;

        for (ent = g_edicts; ent < &g_edicts[globals.num_edicts]; ent++)
        {
                if (!ent->inuse)
			continue;

                /* Kill any fires. */
                ent->burnout = 0;
                if (ent->burner)
                {       ent->burner->enemy = NULL;
                        G_FreeEdict (ent->burner);
                        ent->burner = NULL;
                }

                /* Unlink all firestorms. */
                ent->stormlink = NULL;
        }
}

/*-------------------------------------------------------- New Code --------
//  This checks if we should trade edict/poly heavy effects for
//  fewer edicts and/or quicker explosions.
//------------------------------------------------------------------------*/
qboolean PBM_NeedSpeed (void)
{
/* Net games need to run more smoothly. */
        if (deathmatch->value || coop->value)
                return true;

        return false;
}


/*========================/  Custom Explosions  /========================*/

/*------------------------------------------------------/ New Code /--------
//  This makes some smoke.
//------------------------------------------------------------------------*/
void PBM_MakeSmoke (vec3_t spot)
{
        vec3_t  up = {0, 0, 1};

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_STEAM);
        gi.WriteShort (-1);     // edict id?
        gi.WriteByte (50);      // # of particles
        gi.WritePosition (spot);
        gi.WriteDir (up);
        gi.WriteByte (6);       // Q2 color index
        gi.WriteShort (100);    // speed/width
// Optional:  Write long which is duration in millisec; must have +id.
        gi.multicast (spot, MULTICAST_PVS);
}

/*------------------------------------------------------/ New Code /--------
//  This transforms the fire into smoke.
//------------------------------------------------------------------------*/
void PBM_BecomeSmoke (edict_t *self)
{
        PBM_MakeSmoke (self->s.origin);
        G_FreeEdict (self);
}

/*------------------------------------------------------/ New Code /--------
//  This transforms the fire into steam.
//------------------------------------------------------------------------*/
void PBM_BecomeSteam (edict_t *self)
{
/* world/steam*.wav are standard Q2 sounds. */
	if (rand() & 1)
                gi.sound (self, CHAN_AUTO, gi.soundindex("world/steam1.wav"), 1, ATTN_NORM, 0);
	else
                gi.sound (self, CHAN_AUTO, gi.soundindex("world/steam2.wav"), 1, ATTN_NORM, 0);

        PBM_BecomeSmoke (self);
}

/*------------------------------------------------------/ New Code /--------
//  This animates the small explosion.  The magic numbers in this
//  function represent the frame numbers in id's r_explode md2.
//------------------------------------------------------------------------*/
void PBM_SmallExplodeThink (edict_t *self)
{
        if (++self->s.frame > 7)
	{	G_FreeEdict (self);
		return;
	}

	self->s.skinnum++;

        if (self->s.frame == 7)
                self->s.renderfx |= RF_TRANSLUCENT;
	else if (self->s.frame == 3)
		self->s.effects &= ~EF_HYPERBLASTER;

	self->nextthink = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  This animates the small explosion.  Same as above function
//  except it does not last as long.  Useful for faster play.
//------------------------------------------------------------------------*/
void PBM_FastSmallExplodeThink (edict_t *self)
{
        if (++self->s.frame > 4)
	{	G_FreeEdict (self);
		return;
	}

        self->s.skinnum += 2;

        if (self->s.frame == 4)
                self->s.renderfx |= RF_TRANSLUCENT;
        else if (self->s.frame == 3)
		self->s.effects &= ~EF_HYPERBLASTER;

	self->nextthink = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  This transforms an entity into a small explosion.
//------------------------------------------------------------------------*/
void PBM_StartSmallExplosion (edict_t *self)
{
        self->s.modelindex  = gi.modelindex("models/objects/r_explode/tris.md2");
        self->s.frame       = 1;
        self->s.skinnum     = 0;
        self->touch         = NULL;
        self->solid         = SOLID_NOT;
        self->takedamage    = DAMAGE_NO;
        self->clipmask      = 0;
        self->s.effects     = EF_HYPERBLASTER;
        self->s.renderfx    = RF_FULLBRIGHT;
        self->movetype      = MOVETYPE_NONE;
        VectorClear (self->velocity);
        VectorClear (self->s.angles);
        self->s.angles[YAW] = rand() % 360;
        self->nextthink     = level.time + FRAMETIME;
        if (PBM_NeedSpeed ())
                self->think         = PBM_FastSmallExplodeThink;
        else
                self->think         = PBM_SmallExplodeThink;

        gi.linkentity (self);
}

/*------------------------------------------------------/ New Code /--------
//  This tells an entity to become a small explosion.  If entity
//  is underwater, steam is produced instead.
//------------------------------------------------------------------------*/
void PBM_BecomeSmallExplosion (edict_t *self)
{
        if (gi.pointcontents(self->s.origin) & MASK_WATER)
        {       PBM_BecomeSteam (self);
                return;
        }

/* Play explosion sound at half volume. */
        gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/grenlx1a.wav"), 0.5, ATTN_NORM, 0);

        PBM_StartSmallExplosion (self);
}


/*=============================/  Queries  /=============================*/

/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' (the entity on fire) found some water.
//
//  NOTE:  Checking the entity rather than the flame itself
//         works to the entity's benefit, because the flame
//         is (usually) set higher than ent->s.origin.
//------------------------------------------------------------------------*/
qboolean PBM_InWater (edict_t *ent)
{
/* For players and monsters. */
	if (ent->waterlevel >= 2)
		return true;

/* All-purpose check. */
        if (gi.pointcontents(ent->s.origin) & MASK_WATER)
		return true;

        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' is impervious to fire.  Entities that are
//  immune to fire return true.  Entities that can burn return false.
//------------------------------------------------------------------------*/
qboolean PBM_Inflammable (edict_t *ent)
{
/* Thou canst not burn what doth not exist. */
        if (!ent || !ent->inuse)  return true;

/* Any entity that is impervious to damage is inflammable. */
        if (!ent->takedamage)  return true;

/* Dead and gibbed entities cannot burn. */
        if (ent->health <= ent->gib_health)  return true;

/* Any entity that is immune to lava cannot burn. */
        if (ent->flags & FL_IMMUNE_LAVA)  return true;

/* If entity is a client, check for powerups. */
	if (ent->client)
	{
	/* Invulnerability protects entity from burning. */
		if (ent->client->invincible_framenum > level.framenum)
                        return true;

	/* Bio-suit offers partial immunity to fire. */
                if ((ent->client->enviro_framenum > level.framenum) && (random() < 0.8))
                        return true;
	}

/* The entity is flammable. */
        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' is protected by power armor.
//------------------------------------------------------------------------*/
qboolean PBM_ActivePowerArmor (edict_t *ent, vec3_t point)
{
        int     power_armor_type;

/* Check power armor type. */
        if (ent->client)
		power_armor_type = PowerArmorType (ent);
	else if (ent->svflags & SVF_MONSTER)
        {       if (ent->monsterinfo.power_armor_power <= 0)
                        return false;
                power_armor_type = ent->monsterinfo.power_armor_type;
        }
	else
                return false;

	if (power_armor_type == POWER_ARMOR_NONE)
                return false;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
                vec3_t  forward;
                vec3_t  v;
                float   dot;

                /* Only works if damage point is in front. */
		AngleVectors (ent->s.angles, forward, NULL, NULL);
                VectorSubtract (point, ent->s.origin, v);
                VectorNormalize (v);
                dot = DotProduct (v, forward);
		if (dot <= 0.3)
                        return false;
	}

/* Entity is protected by power armor. */
        return true;
}

/*------------------------------------------------------/ New Code /--------
//  This checks if 'ent' can be set on fire.  If so, false is returned.
//------------------------------------------------------------------------*/
qboolean PBM_FireResistant (edict_t *ent, vec3_t point)
{
/* An entity immune to damage (or fire) obviously can resist fire. */
        if (PBM_Inflammable(ent))
		return true;

/* Check if entity is resistant to fire.  (True by default.) */
        if (!(ent->fireflags & FIREFLAG_IGNITE))
		return true;

/* Check if entity is protected by power armor. */
        if (PBM_ActivePowerArmor (ent, point))
                return true;

/* Entity can be set on fire. */
	return false;
}


/*===========================/  Fire Damage  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This damages a single entity and possibly sets it on fire.
//
//  NOTE(1):  Damage = 1d(die) + base.  (This is D&D notation)
//            Chance is expressed as a percentange.  (e.g., 20 = 20%.)
//
//  NOTE(2):  All fire damage uses the field '(edict)->master'.
//            That field cannot be NULL, or Q2 may crash.
//
//  NOTE(3):  Fire attacks never knockback -- they only burn.
//            Hence, the knockback argument is always '0'.
//------------------------------------------------------------------------*/
void PBM_BurnDamage
(edict_t *victim, edict_t *fire, vec3_t point, vec3_t normal, vec3_t damage, int dflags, int mod)
{
	int   points;
	int   die = (int)damage[0];
	int   base = (int)damage[1];
	int   chance = (int)damage[2];

/* Calculate damage. */
	if (die > 0)
		points = (rand() % die) + base + 1;
	else
		points = base;

/* Check if entity will catch on fire. */
	if ((rand() % 100) < chance)
                PBM_Ignite (victim, fire->master, point);

/* Inflict some burn damage. */
        T_Damage (victim, fire, fire->master, vec3_origin, point, normal, points, 0, dflags, mod);
}

/*------------------------------------------------------/ New Code /--------
//  This damages entities within a certain radius.  There is
//  also a chance that affected entities will catch on fire.
//
//  NOTE:  Damage = 1d(die) + base.  Chance is expressed as percentange.
//------------------------------------------------------------------------*/
void PBM_BurnRadius
(edict_t *fire, float radius, vec3_t damage, edict_t *ignore, int mod)
{
        edict_t *ent = NULL;
        vec3_t  point;

	while ((ent = findradius(ent, fire->s.origin, radius)) != NULL)
	{
                if (!CanDamage (ent, fire))  continue;
                if (PBM_Inflammable(ent))  continue;
                if (ent == ignore)  continue;

                /* Locate damage point. */
                VectorSubtract (ent->s.origin, fire->s.origin, point);
                VectorNormalize (point);
                VectorMA (ent->s.origin, -4, point, point);

                /* Burn the target. */
                PBM_BurnDamage (ent, fire, point, vec3_origin, damage, DAMAGE_RADIUS, mod);
	}
}


/*===========================/  Burn Control  /===========================*/

/*------------------------------------------------------/ New Code /--------
//  This checks if the fire's master (owner) is in Quake2.  If not,
//  this changes the fire's master to itself.  This check is needed
//  so that Quake2 does not crash when a fire tries to burn something
//  after its master is removed from Quake2.
//------------------------------------------------------------------------*/
void PBM_CheckMaster (edict_t *fire)
{
        if (!fire->master)
        {       fire->master = fire;
                fire->owner = fire;
        }
}

/*------------------------------------------------------/ New Code /--------
//  This returns the best location on the entity for where the fire
//  should burn.
//------------------------------------------------------------------------*/
void PBM_FireSpot (vec3_t spot, edict_t *ent)
{
        vec3_t  forward;
        int     delta = 0;

/* Get the entity's forward direction. */
        AngleVectors (ent->s.angles, forward, NULL, NULL);
        VectorNormalize (forward);
        VectorScale (forward, 4, forward);

/* Put the flame at the best spot on an entity. */
        VectorCopy (ent->s.origin, spot);
        VectorAdd (spot, forward, spot);

/* Adjust elevation of flame depending on the entity. */
        if ((ent->health > 0) || (!(ent->fireflags & FIREFLAG_DELTA_ALIVE)))
        {
                delta = ent->fireflags & FIREFLAG_SHIFT_Z;
                if (ent->fireflags & FIREFLAG_DOWN)
                        delta = 0 - delta;
        }

        if (ent->fireflags & FIREFLAG_DELTA_BASE)
        {       /* Dead entities are lower than living ones. */
                if (ent->health > 0)
                        delta += 4;
                else
                        delta -= 18;
        }

        if (ent->fireflags & FIREFLAG_DELTA_VIEW)
                delta += ent->viewheight;

        spot[2] += delta;
}

/*------------------------------------------------------/ New Code /--------
//  This checks if the fire burning on an entity should die.
//------------------------------------------------------------------------*/
qboolean PBM_FlameOut (edict_t *self)
{
/* If entity suddenly disappears from Quake2, remove the fire. */
        if (!self->enemy || !self->enemy->inuse)
        {       PBM_BecomeSmallExplosion (self);
		return true;
	}

/* If no burnout time is specified, entity should not be burning. */
	if (!self->enemy->burnout)
        {       PBM_BecomeSmallExplosion (self);
		return true;
	}

/* Explode as soon as the entity is gibbed. */
	if (self->enemy->health <= self->enemy->gib_health)
	{	self->enemy->burnout = 0;
                PBM_BecomeSmallExplosion (self);
		return true;
	}

/* The fire burns itself out after a period of time. */
	if (self->enemy->burnout < level.time)
	{	self->enemy->burnout = 0;
                PBM_BecomeSmallExplosion (self);
		return true;
	}

/* Check if the victim found some water. */
	if (PBM_InWater(self->enemy))
	{	self->enemy->burnout = 0;
		PBM_BecomeSteam (self);
		return true;
	}

/* Gaining invulnerability kills the fire immediately. */
	if (self->enemy->client)
        {       if (self->enemy->client->invincible_framenum > level.framenum)
		{	self->enemy->burnout = 0;
                        PBM_BecomeSmallExplosion (self);
			return true;
		}
        }

/* Keep on burning! */
        return false;
}

/*------------------------------------------------------/ New Code /--------
//  This keeps the fire on an entity burning.
//
//  NOTE:  Because of client prediction, the client's POV (point of view)
//         is ahead of its body.  So, if the flames burn on the client's
//         body where they should, the flames will lag behind the POV.
//         Giving the fire the client's velocity will often place the
//         fire ahead of the client's body, where it will likely be
//         closer to the POV.  With all this in mind, we give flames
//         velocity only in single-player games, and if the victim is
//         a client with prediction on.
//------------------------------------------------------------------------*/
void PBM_Burn (edict_t *self)
{
	vec3_t   spot;

/* Check if fire should be extinguished. */
        if (PBM_FlameOut (self))
		return;

/* Burn 'em all!  Make fire stick to target. */
        PBM_FireSpot (spot, self->enemy);
        VectorCopy (spot, self->s.origin);
        if (self->enemy->client)
        {       if ( deathmatch->value || coop->value || (self->enemy->client->ps.pmove.pm_flags & PMF_NO_PREDICTION) )
                        VectorClear (self->velocity);
                else
                        VectorCopy (self->enemy->velocity, self->velocity);
        }
        else
                VectorClear (self->velocity);
        gi.linkentity (self);

/* Animate the fire. */
        if (++self->s.frame > FRAME_FIRE_fireb12)
                self->s.frame = FRAME_FIRE_fireb1;

/* Spread the fire! */
	if (self->timestamp < level.time)
        {       PBM_CheckMaster (self);
                PBM_BurnRadius (self, 70, self->pos2, self->enemy, MOD_FIRE_SPLASH);
                PBM_BurnDamage (self->enemy, self, self->enemy->s.origin, vec3_origin, self->pos1, 0, MOD_ON_FIRE);
		self->timestamp = level.time + 1;
	}

	self->nextthink  = level.time + FRAMETIME;
}

/*------------------------------------------------------/ New Code /--------
//  This sets the victim on fire.
//------------------------------------------------------------------------*/
void PBM_Ignite (edict_t *victim, edict_t *attacker, vec3_t point)
{
        edict_t *fire;
        vec3_t  spot;
        vec3_t  burn_damage = {10, 5, 0};
	vec3_t	radius_damage = {0, 5, 50};
        float   duration = BURN_TIME;

/* Some entities vulnerable to fire damage can resist burning. */
        if (PBM_FireResistant (victim, point))  return;

/* Don't set allies on fire if friendly fire is turned off. */
        if (CheckTeamDamage (victim, attacker))  return;

/* Time may be different in deathmatch. */
        if (deathmatch->value)
                duration = BURN_DM_TIME;

/* If entity is already burning, extend time and switch owners. */
        if (victim->burner && (victim->burnout > level.time))
        {       victim->burnout         = level.time + duration;
                victim->burner->owner   = attacker;
                victim->burner->master  = attacker;
                return;
        }

/* Entity will burn for a period of time. */
        victim->burnout = level.time + duration;

/* Create the fire. */
        PBM_FireSpot (spot, victim);

	fire = G_Spawn();
	fire->s.modelindex = MD2_FIRE;
        fire->s.frame      = FRAME_FIRE_igniteb1;
        fire->s.skinnum    = SKIN_FIRE_skin;
	VectorClear (fire->mins);
	VectorClear (fire->maxs);
	VectorCopy (spot, fire->s.origin);
        VectorClear (fire->s.angles);
        VectorClear (fire->velocity);
	fire->solid        = SOLID_NOT;
	fire->takedamage   = DAMAGE_NO;
        fire->movetype     = MOVETYPE_FLY;
	fire->clipmask     = 0;
	fire->s.effects    = EF_ROCKET;
	fire->s.renderfx   = RF_FULLBRIGHT;
	fire->owner        = attacker;
        fire->master       = attacker;
	fire->enemy        = victim;
	fire->classname    = "fire";
        fire->timestamp    = level.time + 1;
	fire->nextthink    = level.time + FRAMETIME;
	fire->think        = PBM_Burn;
	VectorCopy (burn_damage, fire->pos1);
	VectorCopy (radius_damage, fire->pos2);

        gi.linkentity (fire);

/* Link victim to fire. */
	victim->burner = fire;
}


/*===========================/  END OF FILE  /===========================*/
