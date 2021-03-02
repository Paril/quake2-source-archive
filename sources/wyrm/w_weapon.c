#include "g_local.h"
#include "w_fire.h"

void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void Grenade_Explode (edict_t *ent);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

#define TRACKER_DAMAGE_FLAGS	(DAMAGE_NO_POWER_ARMOR | DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK)
#define TRACKER_IMPACT_FLAGS	(DAMAGE_NO_POWER_ARMOR | DAMAGE_ENERGY)
#define TRACKER_DAMAGE_TIME             1.0             // seconds (was 0.5)

void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);

/*
=================
horn_findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
static edict_t *horn_findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
/*
		if (from->solid == SOLID_NOT)
			continue;
*/
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}


void tracker_pain_daemon_think (edict_t *self)
{
	static vec3_t	pain_normal = { 0, 0, 1 };
	int				hurt;

	if(!self->inuse)
		return;

	if((level.time - self->timestamp) > TRACKER_DAMAGE_TIME)
	{
		if(!self->enemy->client)
			self->enemy->s.effects &= ~EF_TRACKERTRAIL;
		G_FreeEdict (self);
	}
	else
	{
		if(self->enemy->health > 0)
		{
//			gi.dprintf("ouch %x\n", self);
			T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, pain_normal,
                                                self->dmg, 0, TRACKER_DAMAGE_FLAGS, MOD_DISRUPTOR);
			
			// if we kill the player, we'll be removed.
			if(self->inuse)
			{
				// if we killed a monster, gib them.
				if (self->enemy->health < 1)
				{
					if(self->enemy->gib_health)
						hurt = - self->enemy->gib_health;
					else
						hurt = 500;

//					gi.dprintf("non-player killed. ensuring gib!  %d\n", hurt);
					T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin,
                                                                pain_normal, hurt, 0, TRACKER_DAMAGE_FLAGS, MOD_DISRUPTOR);
				}

				if(self->enemy->client)
					self->enemy->client->tracker_pain_framenum = level.framenum + 1;
				else
					self->enemy->s.effects |= EF_TRACKERTRAIL;
				
				self->nextthink = level.time + FRAMETIME;
			}
		}
		else
		{
			if(!self->enemy->client)
				self->enemy->s.effects &= ~EF_TRACKERTRAIL;
			G_FreeEdict (self);
		}
	}
}

void tracker_pain_daemon_spawn (edict_t *owner, edict_t *enemy, int damage)
{
	edict_t	 *daemon;

	if(enemy == NULL)
		return;

	daemon = G_Spawn();
	daemon->classname = "pain daemon";
	daemon->think = tracker_pain_daemon_think;
	daemon->nextthink = level.time + FRAMETIME;
	daemon->timestamp = level.time;
	daemon->owner = owner;
	daemon->enemy = enemy;
	daemon->dmg = damage;
}

void tracker_explode (edict_t *self, cplane_t *plane)
{
	vec3_t	dir;

	if(!plane)
		VectorClear (dir);
	else
		VectorScale (plane->normal, 256, dir);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

//	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/disrupthit.wav"), 1, ATTN_NORM, 0);
//	tracker_boom_spawn(self->s.origin);

	G_FreeEdict (self);
}

void tracker_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	damagetime;

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
		if((other->svflags & SVF_MONSTER) || other->client)
		{
			if(other->health > 0)		// knockback only for living creatures
			{
				// PMM - kickback was times 4 .. reduced to 3
				// now this does no damage, just knockback
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
                                                        /* self->dmg */ 0, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_DISRUPTOR);
				
				if (!(other->flags & (FL_FLY|FL_SWIM)))
					other->velocity[2] += 140;
				
				damagetime = ((float)self->dmg)*FRAMETIME;
				damagetime = damagetime / TRACKER_DAMAGE_TIME;
//				gi.dprintf ("damage is %f\n", damagetime);

				tracker_pain_daemon_spawn (self->owner, other, (int)damagetime);
			}
			else						// lots of damage (almost autogib) for dead bodies
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
                                                        self->dmg*4, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_DISRUPTOR);
			}
		}
		else	// full damage in one shot for inanimate objects
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
                                                self->dmg, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_DISRUPTOR);
		}
	}

	tracker_explode (self, plane);
	return;
}

void vectoangles2 (vec3_t value1, vec3_t angles);

void tracker_fly (edict_t *self)
{
	vec3_t	dest;
	vec3_t	dir;
	vec3_t	center;

	if ((!self->enemy) || (!self->enemy->inuse) || (self->enemy->health < 1))
	{
		tracker_explode (self, NULL);
		return;
	}
/*
	VectorCopy (self->enemy->s.origin, dest);
	if(self->enemy->client)
		dest[2] += self->enemy->viewheight;
*/
	// PMM - try to hunt for center of enemy, if possible and not client
	if(self->enemy->client)
	{
		VectorCopy (self->enemy->s.origin, dest);
		dest[2] += self->enemy->viewheight;
	}
	// paranoia
	else if (VectorCompare(self->enemy->absmin, vec3_origin) || VectorCompare(self->enemy->absmax, vec3_origin))
	{
		VectorCopy (self->enemy->s.origin, dest);
	}
	else
	{
		VectorMA (vec3_origin, 0.5, self->enemy->absmin, center);
		VectorMA (center, 0.5, self->enemy->absmax, center);
		VectorCopy (center, dest);
	}

	VectorSubtract (dest, self->s.origin, dir);
	VectorNormalize (dir);
	vectoangles2 (dir, self->s.angles);
	VectorScale (dir, self->speed, self->velocity);
	VectorCopy(dest, self->monsterinfo.saved_goal);

	self->nextthink = level.time + 0.1;
}

void fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles2 (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->speed = speed;
	bolt->s.effects = EF_TRACKER;
//        bolt->s.sound = gi.soundindex ("weapons/disrupt.wav");
        bolt->s.sound = gi.soundindex ("weapons/rockfly.wav");
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	
//        bolt->s.modelindex = gi.modelindex ("models/proj/disintegrator/tris.md2");
        bolt->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	bolt->touch = tracker_touch;
	bolt->enemy = enemy;
	bolt->owner = self;
	bolt->dmg = damage;
	bolt->classname = "tracker";
	gi.linkentity (bolt);

	if(enemy)
	{
		bolt->nextthink = level.time + 0.1;
		bolt->think = tracker_fly;
	}
	else
	{
		bolt->nextthink = level.time + 10;
		bolt->think = G_FreeEdict;
	}

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


// RAFAEL

/*
        fire_pulserifle
*/

void pulserifle_sparks (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (0);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe4 + (rand()&3));
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}

// RAFAEL
void pulserifle_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

        if (self->enemy->client)
                        PlayerNoise (self->enemy, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
                T_Damage (other, self, self->enemy, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_PULSERIFLE);

	}
	else
	{
                self->owner = NULL;
                //bouncing...
                if (plane)
                {
                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_SPARKS);
                //      gi.WriteByte (damage);
                        gi.WritePosition (self->s.origin);
                        gi.WriteDir (plane->normal);
                        gi.multicast (self->s.origin, MULTICAST_PVS);
                }
		return;
	}

	G_FreeEdict (self);
}


// RAFAEL
void fire_pulserifle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
        edict_t *pulse;
	trace_t tr;

	VectorNormalize (dir);

        pulse = G_Spawn ();
        VectorCopy (start, pulse->s.origin);
        VectorCopy (start, pulse->s.old_origin);
        vectoangles (dir, pulse->s.angles);
        VectorScale (dir, speed, pulse->velocity);

        pulse->movetype = MOVETYPE_BOUNCEFLY;
        pulse->clipmask = MASK_SHOT;
        pulse->solid = SOLID_BBOX;
        pulse->s.effects |= effect;

        pulse->s.renderfx |= RF_FULLBRIGHT;

        VectorClear (pulse->mins);
        VectorClear (pulse->maxs);
        pulse->s.modelindex = gi.modelindex ("models/nail/tris.md2");
        pulse->s.sound = gi.soundindex ("misc/lasfly.wav");
        pulse->enemy = pulse->owner = self;
        pulse->touch = pulserifle_touch;
        pulse->nextthink = level.time + 6;
        pulse->think = pulserifle_sparks;
        pulse->dmg = damage;
        pulse->dmg_radius = 100;
        gi.linkentity (pulse);

	if (self->client)
                check_dodge (self, pulse->s.origin, dir, speed);

        tr = gi.trace (self->s.origin, NULL, NULL, pulse->s.origin, pulse, MASK_SHOT);
        if (tr.fraction < 1.0)
	{
                VectorMA (pulse->s.origin, -10, dir, pulse->s.origin);
                pulse->touch (pulse, tr.ent, NULL, NULL);
	}
}


/***************************************************/
/**************    Soldier's Laser    **************/
/***************************************************/
//for tech soldier...
void fire_laser (edict_t *self, vec3_t start, vec3_t aim, int damage)
{
        vec3_t realstart;
	edict_t	*ignore;
	vec3_t	end;
	trace_t	tr;

        gi.sound(self, CHAN_AUTO, gi.soundindex("hover/hovatck1.wav"), 1, ATTN_NORM, 0);

	ignore = self;

        VectorCopy(start, realstart);

        VectorMA (start, 2048, aim, end);
	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
                        T_Damage (tr.ent, self, self, aim, tr.endpos, vec3_origin, damage, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
                                //final effect
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
                                gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
                                gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
                                //laser effect
                                gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_BFG_LASER);
                                gi.WritePosition (realstart);
                                gi.WritePosition (tr.endpos);
                                gi.multicast (start, MULTICAST_PHS);

                                break;
		}

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}
}

/* Misc functions */
void SpawnExplosion( vec3_t start, edict_t *self, float damage, edict_t *ignore, float radius, int mod)
{
        edict_t *xplosion;

        xplosion = G_Spawn();
        VectorCopy (start, xplosion->s.origin);
        T_RadiusDamage(xplosion, self, damage, world, radius, mod);
        G_FreeEdict (xplosion);
        
}


/***************************************************/
/**************       flaregun        **************/
/***************************************************/


/*
 * flare_think
 */

void flare_think(edict_t *self)
{
        // self->timestamp is 15 seconds after the flare was spawned.
        //

        if( level.time > self->timestamp )
        {
                G_FreeEdict( self );
        }
        // We're still active, so lets see if we need to shoot some sparks.
        //
        else
        {
                //If we're in water, we won't spark. Otherwise we will...
                //
                if(gi.pointcontents(self->s.origin) & MASK_WATER)
                {
                        // We're in water, so let's not glow anymore.
                        //
                        self->s.effects = 0;
                        self->s.sound = 0;
                        self->s.renderfx = 0;
                }
                //Check Blindness?
        }

        //move to angles...
        if (VectorCompare(self->velocity, vec3_origin))
        {
                vec3_t dir;

                VectorCopy(self->velocity, dir);
                VectorNormalize(dir);
                vectoangles (dir, self->s.angles);
        }
        else
                self->s.angles[PITCH] = self->s.angles[ROLL] = 0;

        // We'll think again in .1 seconds
        //
        self->nextthink = level.time+0.1;
}

void flare_touch( edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
                return;

        T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 2, DAMAGE_NO_KNOCKBACK, MOD_FIRE_SPLASH);
        
        //Check if can burn...
        if ((ent->s.effects)&&(!PBM_Inflammable(other)))
        {
                PBM_Ignite (other, ent->owner, ent->s.origin);
                //remove self
                G_FreeEdict(ent);
                return;
        }

}

void fire_flaregun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
        edict_t *flare;
        vec3_t dir;
        vec3_t forward, right, up;

        vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        flare = G_Spawn();
        VectorCopy (start, flare->s.origin);
        VectorScale (aimdir, speed, flare->velocity);
        VectorClear (flare->avelocity);
        VectorCopy(dir, flare->s.angles);
        flare->movetype = MOVETYPE_BOUNCE;
        flare->clipmask = MASK_SHOT;
        flare->solid = SOLID_BBOX;
		flare->flags = FL_FLARE;

        flare->s.effects = EF_ROCKET; //make it glow
        flare->s.effects |= EF_COLOR_SHELL;

        if (ctf->value)
        {
                switch (self->client->resp.ctf_team) {
                case CTF_TEAM1:
                                flare->s.renderfx = RF_SHELL_RED;
                        break;
                case CTF_TEAM2:
                                flare->s.renderfx = RF_SHELL_BLUE;
                        break;
                }
        }
        else
                flare->s.renderfx = (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
        
        VectorClear (flare->mins);
        VectorClear (flare->maxs);

        flare->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
        flare->owner = self;
        flare->touch = flare_touch;
        flare->nextthink = FRAMETIME;
        flare->think = flare_think;
        flare->dmg = damage;
        flare->classname = "flare";

        flare->s.sound = gi.soundindex("weapons/flare.wav");
        
        flare->timestamp = level.time + 15.0; //live for 15 seconds
        gi.linkentity (flare);
}

/***************************************************/
/**************       airfist         **************/
/***************************************************/
qboolean horn_infront (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;
	
	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);
	
        if (dot > 0.8)
		return true;
	return false;
}


void fire_airfist (edict_t *self, vec3_t start, vec3_t aimdir, int inDamage, int strenght, float inRange, float recoil)
{
        vec3_t end, delta;
        vec3_t forward, right, up;
        float percent, distance;
        float max_mass = 800; //max mass that can be moved...
        float actual_mass;
        edict_t *ent;
        float eSpeed;
        //recoil!
        trace_t tr;
        vec3_t dir;
        int ldmg;
        int num_traces;

        AngleVectors (aimdir, forward, right, up);

        ent=NULL;
        while ((ent = horn_findradius(ent, start, inRange)) != NULL)
        {
             
             if (visible(self,ent) &&
                horn_infront(self, ent)&&
                ent != self)
             {
                if ((ent->svflags & SVF_DEADMONSTER)&&(ent->movetype == MOVETYPE_NONE))
                {
                        ent->movetype = MOVETYPE_TOSS;
                        ent->s.origin[2] += 10;
                }

                if (ent->movetype != MOVETYPE_NONE &&
                    ent->movetype != MOVETYPE_NOCLIP &&
                    ent->movetype != MOVETYPE_PUSH &&
                    ent->movetype != MOVETYPE_STOP &&
                    strcmp(ent->classname, "bolt") &&
                    strcmp(ent->classname, "item_flag_team1") &&
                    strcmp(ent->classname, "item_flag_team2")
                    )
                {

                        VectorSubtract(ent->s.origin, start, delta);
                        distance = VectorLength(delta);

                        percent = (inRange - distance) / inRange;


                        if (ent->groundentity)
                        {
                                ent->groundentity = NULL;
                                ent->s.origin[2]++;

                                if (delta[2] < 0)
                                        delta[2] = delta[2] / -2;
                                VectorScale (delta, 0.7, delta);
                                if (delta[2] < 100)
                                        delta[2] = 100;
                        }

                        VectorNormalize(delta);

                        if (!ent->mass) actual_mass = 100;
                        else actual_mass = ent->mass;

                        VectorScale(delta, percent * strenght * (max_mass / actual_mass), delta);

                        if (ent->movetype == MOVETYPE_FLYMISSILE)
                        {

                                eSpeed = VectorLength(ent->velocity);
                                VectorNormalize(delta);
                                VectorScale(delta, eSpeed, ent->velocity);
                        }
                        else
                        {
                                
                                VectorMA(ent->velocity, 1, delta, ent->velocity);

                                //do some damage! :P
                    
                                if (ent->takedamage &&
                                    strcmp(ent->classname, "misc_explobox") &&
                                    (!(ent->flags & FL_FROZEN))
                                   )
                                {
                                        // calculate the damage amount
                                        ldmg = percent * inDamage;

                                        T_Damage (ent, self, self, vec3_origin, ent->s.origin, vec3_origin, ldmg, 0, DAMAGE_NO_KNOCKBACK, MOD_AIRFIST);
                                }        
                        }

                }
             }
        }

        // check if near a wall
        VectorAdd (self->s.origin, self->client->ps.viewoffset, dir);
        VectorMA(dir, 64, forward, end);
        tr = gi.trace (dir, NULL, NULL, end, self, MASK_SOLID);

        if (tr.fraction != 1)
                recoil += recoil * (1-tr.fraction) * 3;

        if (self->groundentity)
        {
                self->groundentity = NULL;
                self->s.origin[2]++;
        }

        //apply recoil
        VectorMA(self->velocity, -recoil, forward, self->velocity);

        //effect!
        gi.sound (self, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex ("chick/chkatck2.wav"), 1, ATTN_NORM, 0);

       num_traces = 0;

       while (num_traces < 45)
       {
             VectorMA(start, 170, forward, end);
             VectorMA(end, 35*crandom(), right, end);
             VectorMA(end, 35*crandom(), up, end);

             gi.WriteByte (svc_temp_entity);
             gi.WriteByte (TE_BUBBLETRAIL);
             gi.WritePosition (start);
             gi.WritePosition (end);
             gi.multicast (start, MULTICAST_PHS);
             num_traces++;
       }


}

/***************************************************/
/**************    double impact      **************/
/***************************************************/
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
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
        grenade->nextthink = level.time + timer;
        grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
        grenade->classname = "dimpact";

       gi.linkentity (grenade);
}


/***************************************************/
/**************  Explosive Machinegun **************/
/***************************************************/
/*
=================
fire_explosive_lead

This is an internal support routine used for explosive bullet/pellet based weapons.
=================
*/
static void fire_explosive_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
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
                                        gi.WriteByte (svc_temp_entity);
                                        gi.WriteByte (te_impact);
                                        gi.WritePosition (tr.endpos);
                                        gi.WriteDir (tr.plane.normal);
                                        gi.multicast (tr.endpos, MULTICAST_PVS);

                                        T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, 8, kick, DAMAGE_BULLET, mod);
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
                        SpawnExplosion(tr.endpos, self, damage, world, damage + 40, mod);
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
fire_explosive_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_explosive_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
        fire_explosive_lead (self, start, aimdir, damage, kick, TE_BLASTER, hspread, vspread, mod);
}

/***************************************************/
/**************    FlameThrower       **************/
/***************************************************/
void flame_think (edict_t *ent)
{
        ent->nextthink = level.time + .1;

        if (PBM_InWater(ent))
        {       PBM_BecomeSteam(ent);
		return;
	}

        if (ent->s.frame < 13)
        {
                ent->s.frame++;
                if (random() > .7) ent->s.frame++;

//                ent->s.skinnum = (ent->s.frame - 1) / 2;
                if (ent->s.frame >= 10)
                {
                        ent->s.renderfx |= RF_TRANSLUCENT;
                        ent->s.renderfx &= ~RF_FULLBRIGHT;
                }
        }
        else
        {
                PBM_BecomeSmoke(ent);
                return;
        }
}

void flame_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	{
                T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, 20, 0, 0, MOD_FIRE_SPLASH);
	}

	G_FreeEdict (ent);
}



void fire_flame (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
        edict_t *flame;

        flame = G_Spawn();
        VectorCopy (start, flame->s.origin);
        VectorCopy (dir, flame->movedir);
        vectoangles (dir, flame->s.angles);
        flame->s.angles[2] = 90;
        flame->s.angles[1] -= 90;
        flame->avelocity[0] = crandom()*90;
        flame->s.angles[0] = crandom()*90;
        VectorScale (dir, speed, flame->velocity);
        flame->movetype = MOVETYPE_FLYMISSILE;
        flame->clipmask = MASK_SHOT;
        flame->solid = SOLID_BBOX;
        flame->s.renderfx = RF_FULLBRIGHT;
        flame->s.effects = 0;
        VectorClear (flame->mins);
        VectorClear (flame->maxs);
        flame->s.modelindex = gi.modelindex ("models/objects/r_explode/tris.md2");
        flame->master = flame->owner = self;
        flame->touch = PBM_FireballTouch;
        flame->s.frame = 0;
        if (random() < 0.6)
                flame->s.skinnum = 3;
        else
                if (random() < 0.5)
                        flame->s.skinnum = 2;
                else
                        flame->s.skinnum = 4;

        flame->nextthink = level.time + .2;
        flame->think = flame_think;
        flame->dmg = damage;
        flame->s.sound = gi.soundindex ("weapons/rockfly.wav");
        flame->classname = "flame";

        flame->pos1[0] = 6;
        flame->pos1[1] = 9;
        flame->pos1[2] = 25;
        flame->pos2[0] = 6;
        flame->pos2[1] = 4;
        flame->pos2[2] = 25;
        flame->dmg_radius = 70;
        flame->count        = 15;
        flame->dmg          = 10;


	if (self->client)
                check_dodge (self, flame->s.origin, dir, speed);

        gi.linkentity (flame);

        if (PBM_InWater(flame))
        {       PBM_BecomeSteam(flame);
		return;
	}
}

/***************************************************/
/**************  Cluster Grenades     **************/
/***************************************************/
void fire_minigrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
        VectorSet(grenade->velocity, 0, 0, speed);
        VectorAdd(grenade->velocity, aimdir, grenade->velocity);
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
        grenade->nextthink = level.time + timer;
        grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
        grenade->classname = "minigrenade";

       gi.linkentity (grenade);
}

//modified GrenadeExplode for the Cluster
static void ClusterGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;
        int j;
        float rotation, speed;
        vec3_t direction;

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
                mod = MOD_CLUSTERGRENADES;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

        mod = MOD_CLUSTERGRENADES;
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

        for (j=0 ; j <= 5 ; j++)
        {
                rotation = j * 2 * M_PI/5;
                direction[0] = cos(rotation);
                direction[1] = sin(rotation);
                direction[2] = 0;
                VectorNormalize(direction);
                VectorScale(direction, 50, direction);
				speed = 400 + crandom()*90;
                fire_minigrenade (ent->owner, ent->s.origin, direction, 75, speed, 1+crandom()*0.5, 125);
        }

	G_FreeEdict (ent);
}

//modified GrenadeTouch for the Cluster
static void ClusterGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

        //wyrm--> double impact...
        if (other->owner == ent->owner)
                if (!strcmp(other->classname, "dimpact"))
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
                        gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
        ClusterGrenade_Explode (ent);
}


void fire_clustergrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
        grenade->touch = ClusterGrenade_Touch;
        grenade->nextthink = level.time + timer;
        grenade->think = ClusterGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
        grenade->classname = "cluster";

       gi.linkentity (grenade);
}

/***************************************************/
/**************  Guided Misiles       **************/
/***************************************************/
void RemoveGuided(edict_t *ent)
{
        if (!ent->client->missile) return;

        ent->client->missile = NULL;

        if (!ent->client->chasetoggle && !ent->client->onturret)
        {
                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                if (!ent->deadflag)
                        ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
                if (ent->client->oldplayer && ent->client->oldplayer->inuse)
                {
                        ent->s.modelindex = ent->client->oldplayer->s.modelindex;
                        free(ent->client->oldplayer->client); //needed?
                        G_FreeEdict(ent->client->oldplayer);
                }
        }
}

//Explode rocket without touching anything
static void GuidedRocket_Explode (edict_t *ent)
{
       if (ent->owner->client)
       {
               PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
               RemoveGuided(ent->owner);
       }

      // calculate position for the explosion entity

//       T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_GUIDEDROCKETS);
       SpawnExplosion(ent->s.origin, ent->owner, 100, world, 140, MOD_GUIDEDROCKETS);

       gi.WriteByte (svc_temp_entity);
       if (ent->waterlevel)
               gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
       else
               gi.WriteByte (TE_ROCKET_EXPLOSION);
       gi.WritePosition (ent->s.origin);
       gi.multicast (ent->s.origin, MULTICAST_PVS);

       G_FreeEdict (ent);
}
// When a rocket 'dies', it blows up next frame
void GuidedRocket_Think (edict_t *self)
{
       VectorClear(self->velocity);
       self->takedamage = DAMAGE_NO;
       self->nextthink = level.time + .1;
       self->think = GuidedRocket_Explode;
}


void guidedrocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;


        if (surf && (surf->flags & SURF_SKY))
	{
                if (ent->owner->client)
                        RemoveGuided(ent->owner);
		G_FreeEdict (ent);
		return;
	}

        GuidedRocket_Think(ent);
        return;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
                T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_GUIDEDROCKETS);
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

        T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_GUIDEDROCKETS);


	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_guidedrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
        edict_t *rocket;

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
        rocket->touch = guidedrocket_touch;
	rocket->nextthink = level.time + 8000/speed;
        rocket->think = GuidedRocket_Think;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";


	if (self->client)
        {
                self->client->missile = rocket;
                self->client->ps.gunindex = 0;
                if (!self->client->chasetoggle) self->client->oldplayer = G_Spawn();
                self->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

		check_dodge (self, rocket->s.origin, dir, speed);
        }

	gi.linkentity (rocket);
}

/***************************************************/
/**************      Freeze gun       **************/
/***************************************************/
//ice functions
void ThrowIce (edict_t *self)
{
	vec3_t	forward, right;
        vec3_t  start, offset;

        if (!self->client->iceball)
                return;

        AngleVectors (self->client->v_angle, forward, right, NULL);
        VectorSet(offset, 20 + self->maxs[0], 4, self->viewheight - 4);
        P_ProjectSource (self->client, self->s.origin, offset, forward, right, start);

        VectorCopy(start, self->client->iceball->s.origin);
        VectorScale(forward, 800, self->client->iceball->velocity);
        VectorCopy(self->client->iceball->velocity, self->client->iceball->movedir);

        self->client->iceball->prethink = NULL;
        self->client->iceball->think = G_FreeEdict;
        self->client->iceball->nextthink = level.time + 2;

	if (self->client)
                check_dodge (self, self->client->iceball->s.origin, forward, 800);

        self->client->iceball = NULL;
        //repeat ball
        self->client->machinegun_shots = 0;
}

void IcePosFromPlayer (edict_t *ent, float size, vec3_t output)
{
	vec3_t	forward, right;
        vec3_t  offset;

        AngleVectors (ent->client->v_angle, forward, right, NULL);
        VectorSet(offset, 20 + size, 4, ent->viewheight - 4);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, output);

}

void IceThink (edict_t *self)
{
        vec3_t start;

        if (self->owner->flags & FL_FROZEN || (self->owner->deadflag) || (self->owner->movetype == MOVETYPE_NOCLIP))
        {
                ThrowIce(self->owner);
                return;
        }

        IcePosFromPlayer(self->owner, self->maxs[0], start);

        VectorCopy( start , self->s.origin);
        VectorCopy(self->movedir, self->velocity);
}

void UnfreezeMonster (edict_t *self)
{
        int n;

        //just unfreeze the monster
        self->flags &= ~FL_FROZEN;
        //sound
        gi.sound (self, CHAN_AUTO, gi.soundindex ("weapons/shatter.wav"), 1, ATTN_IDLE, 0);

        n = rand() % 4 + 1;
        while(n--)
                ThrowDebris (self, "models/objects/debris2/tris.md2", 1, self->s.origin);

        n = rand() % 3 + 1;
        while(n--)
                ThrowDebris (self, "models/objects/debris1/tris.md2", 1, self->s.origin);

        //Remove Shell...
        self->s.effects = 0;
        self->s.renderfx = 0;

        if (self->flags & FL_WASFLYING)
        {
                self->flags &= ~FL_WASFLYING;
                self->flags |= FL_FLY;
                self->movetype = MOVETYPE_STEP;
        }

        self->think = self->reallythink;
        self->reallythink = NULL;

        if (self->think != NULL)
                self->think(self);
}
#if 0
void AttachIceThink (edict_t *self)
{
        vec3_t spot;

        if (!(self->owner->flags & FL_FROZEN))
        {
                G_FreeEdict(self);
                return;
        }

        if (gi.pointcontents (self->s.origin) & MASK_WATER)
        {
                if (self->owner->client)
                        self->reallynextthink = level.time + FRAMETIME;
                else
                        if (self->owner->think)
                                self->owner->nextthink = level.time + FRAMETIME;
        }

        // I will use som PM fire stuff to attach ice

        PBM_FireSpot (spot, self->owner);
        
        VectorCopy (spot, self->s.origin);
        VectorCopy (self->owner->velocity, self->velocity);
        gi.linkentity (self);

        self->think = AttachIceThink;
        self->nextthink = level.time + FRAMETIME;
}

void AttachIce (edict_t *self)
{
        edict_t *icecube;

        icecube = G_Spawn();
        icecube->owner = self;
        icecube->solid = SOLID_NOT;
        VectorClear(icecube->velocity);
        icecube->s.modelindex = gi.modelindex("models/icecube/tris.md2");
        icecube->s.frame = 1;
        icecube->s.renderfx = RF_TRANSLUCENT;
        icecube->s.effects = EF_FLAG2;

        AttachIceThink(icecube);
        gi.linkentity(icecube);
}
#endif

void T_FreezeRadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
        float   enemyhealth, points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

        while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
                if (!( (ent->svflags & SVF_MONSTER) || (ent->client) ))
                        continue;
                if (ent->health <= 0)
                        continue;
		if (!ent->takedamage)
			continue;
                if (ent->flags & FL_FROZEN)
                        continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
                points = damage - 0.85 * VectorLength (v);
                if (ent == attacker)
                        points = points * 0.5;

                enemyhealth = ent->health;

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
                                VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
                                T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, points, points, DAMAGE_RADIUS|DAMAGE_NO_ARMOR|DAMAGE_NO_KNOCKBACK, mod);

                                if (ent->health == 1)
                                {
                                        ent->flags |= FL_FROZEN;
                                        gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/frozen.wav"), 1, ATTN_NORM, 0);
                                        //attach cube (REMOVED)
                                        //AttachIce(ent);
                                        if (ent->svflags & SVF_MONSTER)
                                        {
                                                //kill his sounds
                                                ent->s.sound = 0;

                                                ent->reallythink = ent->think;
                                                ent->think = UnfreezeMonster;
                                                ent->nextthink = level.time + 4 + 2*random();

                                                //Add shell..
                                                ent->s.effects |= EF_COLOR_SHELL;
                                                ent->s.renderfx |= RF_SHELL_GREEN;
                                                ent->s.renderfx |= RF_SHELL_BLUE;
                                                ent->s.renderfx |= RF_SHELL_RED;

                                                if (ent->flags & FL_FLY)
                                                {
                                                        ent->flags |= FL_WASFLYING;
                                                        ent->groundentity = NULL;
                                                        ent->flags &= ~FL_FLY;
                                                        ent->movetype = MOVETYPE_TOSS;
                                                }
                                        }
                                        else if (ent->client) /*is a client*/
                                        {
                                                //kill sounds
                                                ent->s.sound = 0;
                                                ent->client->weapon_sound = 0;

                                                ent->reallynextthink = level.time + 4 + 2*random();
                                                //save v_angle
                                                //VectorCopy(ent->client->v_angle, ent->monsterinfo.startangles);
                                                //launch his ball...
                                                ThrowIce(ent);
                                        }
                                }
			}
		}
	}
}

void IceTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
        {
                if (other->client)
                if (!other->client->iceball)
                        VectorCopy(ent->movedir, ent->velocity);

		return;
        }

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

        //do some fun stuff...

        
        T_FreezeRadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg + 40, MOD_FREEZE);

        if (ent->owner->client)
        if (ent->owner->client->iceball == ent)
        {
                ent->owner->client->iceball = NULL;
                ent->owner->client->machinegun_shots = 1;
        }

        ent->think = G_FreeEdict;
        ent->nextthink = level.time + .1;
        ent->solid = SOLID_NOT;
        ent->s.event = EV_PLAYER_TELEPORT; //fog!
}

void IceThink (edict_t *self);

void CreateIce (edict_t *self)
{
        vec3_t  start;
        trace_t tr;
        edict_t *ice;

        IcePosFromPlayer(self, 4, start);

        tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
        
        if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
                self->client->pers.inventory[self->client->ammo_index]--;

        ice = G_Spawn();
        self->client->iceball = ice;

        VectorClear(ice->velocity);
        ice->movetype = MOVETYPE_FLYMISSILE;
        ice->clipmask = MASK_SHOT;
        ice->solid = SOLID_BBOX;
        ice->s.effects = EF_COLOR_SHELL|EF_FLAG2;
        ice->s.renderfx = RF_TRANSLUCENT|RF_SHELL_BLUE;
        VectorSet(ice->mins, -4, -4, -4);
        VectorSet(ice->maxs, 4, 4, 4);
        ice->s.modelindex = gi.modelindex ("models/ice/tris.md2");
        ice->owner = self;
        ice->touch = IceTouch;
        ice->s.frame = 0;
        ice->s.skinnum = 0;
        VectorClear(ice->movedir);

        ice->classname = "ice";

        ice->dmg          = 40;

        VectorCopy(tr.endpos, ice->s.origin);

        gi.linkentity (ice);
}

/***************************************************/
/****************     Plasma Gun     ***************/
/***************************************************/

void plasma_explode (edict_t *ent)
{
	int			mod;


        mod = MOD_PLASMA;
	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

        ent->dmg *= 2;

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
        T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg + 40, mod);

        G_FreeEdict (ent);
}

void plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int mod;

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
                mod = MOD_PLASMA;
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


void plasma_think (edict_t *self)
{
        if (level.time > self->timeout)
                G_FreeEdict(self);
        else if (gi.pointcontents (self->s.origin) & MASK_WATER)
                plasma_explode(self);
        else
                self->nextthink = level.time + FRAMETIME;
}


void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, vec3_t up, vec3_t right, int damage, int speed)
{
        edict_t *plasma;

        plasma = G_Spawn();
        VectorCopy (start, plasma->s.origin);
        VectorCopy (dir, plasma->movedir);
        vectoangles (dir, plasma->s.angles);
        VectorScale (dir, speed, plasma->velocity);
        VectorMA(plasma->velocity, crandom() * 50, up, plasma->velocity);
        VectorMA(plasma->velocity, crandom() * 50, right, plasma->velocity);

        VectorSet (plasma->avelocity, 300, 300, 300);

        plasma->movetype = MOVETYPE_FLYMISSILE;
        plasma->clipmask = MASK_SHOT;
        plasma->solid = SOLID_BBOX;
        plasma->s.effects |= EF_BLUEHYPERBLASTER;
        VectorClear (plasma->mins);
        VectorClear (plasma->maxs);
        plasma->s.modelindex = gi.modelindex ("models/pbullet/tris.md2");
        plasma->owner = self;
        plasma->touch = plasma_touch;
        plasma->nextthink = level.time + 8000/speed;
        plasma->think = G_FreeEdict;
        plasma->radius_dmg = damage;
        plasma->dmg = damage;
        plasma->classname = "plasma bullet";
        plasma->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

        plasma->think = plasma_think;
        
        plasma->timeout = level.time + 5;
        plasma->teammaster = plasma;
        plasma->teamchain = NULL;

        VectorCopy(plasma->velocity, dir);
        VectorNormalize(dir);
	if (self->client)
                check_dodge (self, plasma->s.origin, dir, speed);

        gi.linkentity (plasma);

        plasma_think (plasma); //check if in water
}


/***************************************************/
/**************  Anti-Matter Cannon   **************/
/***************************************************/
void BombExplosion (edict_t *self)
{
        T_RadiusDamage(self, self->owner, self->dmg, self->enemy, self->dmg_radius, MOD_ANTIMATTER);

	gi.WriteByte (svc_temp_entity);
        if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PHS);

        G_FreeEdict (self);
        
}

void fire_asha  (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
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
        qboolean        is_onturret=false;

//eo! Turret encara no posat! FIXME
#if 0
        if (self->client)
                is_onturret = (self->client->onturret > 0);
        else
                if (self->flags & FL_TEAMSLAVE)
                        is_onturret=true;
#endif

        if (!is_onturret)
                tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
        else
                tr = gi.trace (start, NULL, NULL, start, self, MASK_SHOT);

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
                        edict_t *bomb;
                        vec3_t bdest;

			if (tr.ent->takedamage)
			{
                                T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, mod);
			}
                        
                        VectorMA(tr.endpos, -4, aimdir, bdest);
                        bomb = G_Spawn();
                        bomb->think = BombExplosion;
                        bomb->dmg = 80;
                        bomb->enemy = tr.ent;
                        bomb->dmg_radius = 100;
                        bomb->nextthink = level.time + 0.1 + random() * 0.3;
                        VectorCopy(bdest, bomb->s.origin);
                        bomb->svflags = SVF_NOCLIENT;
                        bomb->owner = self;

                        gi.linkentity(bomb);
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


/***************************************************/
/**************     Positron Beam     **************/
/***************************************************/
void Shake (edict_t *self)
{
        edict_t *selected;
        vec3_t dist;
        float distance;
        int i;

	for (i=0 ; i<maxclients->value ; i++)
	{
                selected = g_edicts + 1 + i;
                if (!selected->inuse)
			continue;

                if (visible(self, selected))
                {
                        VectorSubtract(selected->s.origin, self->s.origin, dist);
                        if ((distance=VectorLength(dist)) <= 500)
                                selected->client->kick_angles[0] = crandom() * (10 - distance * 0.02);
                }
        }
}

void Positron_Think (edict_t *self)
{
        if (self->s.frame > 17)
        {
                G_FreeEdict (self);
                return;
        }
        Shake (self);
        T_RadiusDamage(self, self->owner, (12 * self->s.frame), self->enemy, (12 * self->s.frame + 40), MOD_POSITRON);
        //   T_RadiusDamage (self,self.owner,(8 * self.frame),world);
        if (!(self->s.frame % 3))
                self->s.frame += 2;
        else
                self->s.frame++;
        self->nextthink = level.time + FRAMETIME;
};

void Positron_Exp (edict_t *self)
{
/*
        self->avelocity[0] = random() * 100 + 100;
        if (random() < 0.5) self->avelocity[0] = -self->avelocity[0];
        self->avelocity[1] = random() * 100 + 100;
        if (random() < 0.5) self->avelocity[1] = -self->avelocity[1];
        self->avelocity[2] = random() * 100 + 100;
        if (random() < 0.5) self->avelocity[2] = -self->avelocity[2];
*/
        self->avelocity[1] = 200;
        self->avelocity[2] = self->avelocity[0] = 0;

        self->s.modelindex = gi.modelindex("models/positron/tris.md2");
        self->s.frame = 0;
        self->s.skinnum = 0;
        self->s.effects = EF_HYPERBLASTER;
        self->s.renderfx = RF_TRANSLUCENT;
        gi.linkentity(self);
        self->think = Positron_Think;
        self->nextthink = level.time + FRAMETIME;
        gi.sound (self, CHAN_AUTO, gi.soundindex ("weapons/pt_blast.wav"), 1, ATTN_NORM, 0);
};

void MakePositron (edict_t *maker, edict_t *ignore, vec3_t origin, float delay)
{
        edict_t *positron;

        positron = G_Spawn();
        positron->owner = maker;
        VectorCopy(origin, positron->s.origin);
        VectorClear (positron->velocity);
        positron->movetype = MOVETYPE_FLYMISSILE;
        positron->s.modelindex = 0;
        positron->think = Positron_Exp;
        positron->nextthink = level.time + delay + FRAMETIME;
        positron->takedamage = DAMAGE_NO;
        positron->enemy = ignore;
        gi.linkentity (positron);
};


void PositronBeamFire(edict_t *ent, vec3_t start, vec3_t dir)
{
        vec3_t end;
        trace_t tr;

        VectorMA (start, 2048, dir, end);

        tr = gi.trace (start, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

        //Beam Effect
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BFG_LASER);
        gi.WritePosition (start);
        gi.WritePosition (tr.endpos);
        gi.multicast (start, MULTICAST_PHS);

	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
        {
                if (tr.fraction < 1.0)
                {
                        if ((tr.ent) && (tr.ent->takedamage))
                                MakePositron(ent, NULL, tr.endpos, 0);
                        else
                                MakePositron(ent, NULL, tr.endpos, 0.4);
                }
                else
                        MakePositron(ent, NULL, tr.endpos, 0.4);
        }
}

/***************************************************/
/**************     Shell Ejection    **************/
/***************************************************/
void removeOldestShelcase()
{
	edict_t *oldestEnt = NULL;
	edict_t *e = NULL;
	int count = 0;
        int max_shells = maxshells->value;

	while(1)
	{
                e = G_Find(e, FOFS(classname), "shelcase");
		if (e == NULL) // no more
			break;

		count++;

		if (oldestEnt == NULL ||
			e->timestamp < oldestEnt->timestamp)
		{
			oldestEnt = e;
		}
	}
        if (count > max_shells && oldestEnt != NULL)
                G_FreeEdict(oldestEnt);
}

void shell_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other->takedamage == DAMAGE_AIM)
                return;

        VectorClear(self->avelocity);
        self->s.angles[0] = self->s.angles[2] = 0;
}

void eject_shell (edict_t *ent, vec3_t org, vec3_t dir)
{
        edict_t *shell;
        vec3_t aimdir;

        if (maxshells->value)
        {
                shell = G_Spawn();

                shell->owner = ent;
                shell->touch = shell_touch;

                shell->classname = "shelcase";
                shell->timestamp = level.time;

                shell->s.modelindex = gi.modelindex("models/shell/tris.md2");
                shell->s.frame = 0;

                shell->movetype = MOVETYPE_BOUNCE;
                shell->solid = SOLID_BBOX;

                if (ent->client)
                        VectorCopy(ent->client->v_angle, aimdir);
                else
                        vectoangles(dir, aimdir);
        
                VectorCopy(aimdir, shell->s.angles);
                VectorCopy(dir, shell->velocity);

                VectorClear(shell->maxs);
                VectorClear(shell->mins);

                VectorCopy(org, shell->s.origin);

                VectorClear(shell->avelocity);

                if (tempents_timelimit->value)
                {
                        shell->think = G_FreeEdict;
                        shell->nextthink = level.time + 2 + 2*random();
                }

                gi.linkentity (shell);
        }

        removeOldestShelcase();
}


/***************************************************/
/**************     Stick Function    **************/
/***************************************************/
void Stick(edict_t *ent, edict_t *other)
{

                if (ent->movetype == MOVETYPE_STUCK)
                        return;

// if we're sticking to a monster or a client, stick closer....
                if (/*(other->client) ||*/ (other->svflags & SVF_MONSTER))
                {
                        vec3_t forward;
                        float addum=25;
                        /*
                        VectorSubtract(other->maxs, other->mins, temp);
                        addum = sqrt(sqrt(fabs(temp[0]*temp[1]*temp[2])));
                        */
                        AngleVectors (ent->s.angles, forward, NULL, NULL);
                        VectorMA(ent->s.origin, addum, forward, ent->s.origin);
                }

                ent->movetype = MOVETYPE_STUCK;
                VectorClear(ent->velocity);
                VectorClear(ent->avelocity);
                ent->stuckentity = other;
                VectorCopy(other->s.origin,ent->oldstuckorigin);
                VectorCopy(other->s.angles,ent->oldstuckangles);
}

/*************************************************/
/**************     Bullet Holes    **************/
/*************************************************/
void removeOldestBullethole()
{
	edict_t *oldestEnt = NULL;
	edict_t *e = NULL;
	int count = 0;
        int max_bulletholes = maxbulletholes->value;

	while(1)
	{
                e = G_Find(e, FOFS(classname), "bullethole");
		if (e == NULL) // no more
			break;

		count++;

		if (oldestEnt == NULL ||
			e->timestamp < oldestEnt->timestamp)
		{
			oldestEnt = e;
		}
	}
        if (count > max_bulletholes && oldestEnt != NULL)
                G_FreeEdict(oldestEnt);
}

void create_bullethole (edict_t *target, vec3_t org, vec3_t dir)
{
        edict_t *bullethole;

        if (maxbulletholes->value)
        {
                bullethole = G_Spawn();

                bullethole->classname = "bullethole";
                bullethole->timestamp = level.time;

                bullethole->s.modelindex = gi.modelindex("models/objects/bullethole/tris.md2");
                bullethole->s.frame = 0;

                bullethole->movetype = MOVETYPE_NONE;
                bullethole->solid = SOLID_NOT;

                vectoangles(dir, bullethole->s.angles);
                bullethole->s.angles[0] += 90;

                VectorClear(bullethole->maxs);
                VectorClear(bullethole->mins);

                VectorCopy(org, bullethole->s.origin);

                VectorClear(bullethole->avelocity);

                if (tempents_timelimit->value)
                {
                        bullethole->think = G_FreeEdict;
                        bullethole->nextthink = level.time + 4 + 2*random();
                }

                Stick(bullethole, target);

                gi.linkentity (bullethole);
        }

        removeOldestBullethole();
}


/**********************************************/
/**************     BFGrenade    **************/
/**********************************************/

/*
=================
fire_bfgrenade
=================
*/
void bfg_explode (edict_t *self);

static void BFGrenade_Explode (edict_t *ent)
{
        if (ent->owner->client)
                PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

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
                T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, 0, 0, MOD_BFGRENADES);
	}

	// core explosion - prevents firing it into the wall/floor
        T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, 100, MOD_BFGRENADES);

        gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
        gi.sound (ent, CHAN_ITEM, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
        ent->solid = SOLID_NOT;
        ent->movetype = MOVETYPE_NONE;
        ent->touch = NULL;
/*
        VectorMA (ent->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
*/
        ent->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
        ent->s.frame = 0;
        ent->s.sound = 0;
        ent->radius_dmg = 200;
        ent->dmg_radius = 800;
        ent->think = bfg_explode;
        ent->nextthink = level.time + FRAMETIME;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
        gi.WritePosition (ent->s.origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

}

static void BFGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
        BFGrenade_Explode (ent);
}

void fire_bfgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
        grenade->s.effects |= EF_COLOR_SHELL;
        grenade->s.renderfx |= RF_SHELL_GREEN;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
        grenade->touch = BFGrenade_Touch;
        grenade->nextthink = level.time + timer;
        grenade->think = BFGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}


/*
=================
fire_proxgrenade
=================
*/

#define PROX_TIME_TO_LIVE	45		// 45, 30, 15, 10
#define PROX_TIME_DELAY		0.5
#define PROX_BOUND_SIZE		96
#define PROX_DAMAGE_RADIUS	192
#define PROX_HEALTH			20
#define	PROX_DAMAGE			90

//===============
//===============
void Prox_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t		*owner;

// free the trigger field

	//PMM - changed teammaster to "mover" .. owner of the field is the prox
	if(ent->teamchain && ent->teamchain->owner == ent)
		G_FreeEdict(ent->teamchain);

	owner = ent;
	if(ent->teammaster)
	{
		owner = ent->teammaster;
		PlayerNoise(owner, ent->s.origin, PNOISE_IMPACT);
	}

	// play quad sound if appopriate
	if (ent->dmg > PROX_DAMAGE)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

	ent->takedamage = DAMAGE_NO;
        T_RadiusDamage(ent, owner, ent->dmg, ent, PROX_DAMAGE_RADIUS, MOD_PROXGRENADES);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->groundentity)
		gi.WriteByte (TE_GRENADE_EXPLOSION);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

//===============
//===============
void prox_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	gi.dprintf("prox_die\n");
	// if set off by another prox, delay a little (chained explosions)
	if (strcmp(inflictor->classname, "prox"))
	{
		self->takedamage = DAMAGE_NO;
		Prox_Explode(self);
	}
	else
	{
		self->takedamage = DAMAGE_NO;
		self->think = Prox_Explode;
		self->nextthink = level.time + FRAMETIME;
	}
}

//===============
//===============
void Prox_Field_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *prox;

	if (!(other->svflags & SVF_MONSTER) && !other->client)
		return;

        //if dead...
        if (other->health <= 0)
                return;

	// trigger the prox mine if it's still there, and still mine.
	prox = ent->owner;

	if (other == prox) // don't set self off
		return;

	if (prox->think == Prox_Explode) // we're set to blow!
	{
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf ("%f - prox already gone off!\n", level.time);
		return;
	}

	if(prox->teamchain == ent)
	{
//                gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/proxwarn.wav"), 1, ATTN_NORM, 0);
		prox->think = Prox_Explode;
		prox->nextthink = level.time + PROX_TIME_DELAY;
		return;
	}

	ent->solid = SOLID_NOT;
	G_FreeEdict(ent);
}

void Prox_Field_Think (edict_t *ent)
{
        VectorCopy(ent->owner->s.origin, ent->s.origin);
        ent->nextthink = level.time + FRAMETIME;
}

//===============
//===============
void prox_seek (edict_t *ent)
{
	if(level.time > ent->wait)
		Prox_Explode(ent);
	else
                ent->nextthink = level.time + FRAMETIME;
}

//===============
//===============
void prox_open (edict_t *ent)
{
	edict_t *search;

	search = NULL;

		// set the owner to NULL so the owner can shoot it, etc.  needs to be done here so the owner
		// doesn't get stuck on it while it's opening if fired at point blank wall
		ent->s.sound = 0;
		ent->owner = NULL;
		if(ent->teamchain)
			ent->teamchain->touch = Prox_Field_Touch;
		while ((search = findradius(search, ent->s.origin, PROX_DAMAGE_RADIUS+10)) != NULL)
		{
			if (!search->classname)			// tag token and other weird shit
				continue;

			// if it's a monster or player with health > 0
			// or it's a player start point
			// and we can see it
			// blow up
			if (
				(
					(((search->svflags & SVF_MONSTER) || (search->client)) && (search->health > 0))	|| 
					(
						(deathmatch->value) && 
						(
						(!strcmp(search->classname, "info_player_deathmatch")) ||
						(!strcmp(search->classname, "info_player_start")) ||
						(!strcmp(search->classname, "info_player_coop")) ||
						(!strcmp(search->classname, "misc_teleporter_dest"))
						)
					)
				) 
				&& (visible (search, ent))
			   )
			{
				Prox_Explode (ent);
				return;
			}
		}

			switch (ent->dmg/PROX_DAMAGE)
			{
				case 1:
					ent->wait = level.time + PROX_TIME_TO_LIVE;
					break;
				case 2:
					ent->wait = level.time + 30;
					break;
				case 4:
					ent->wait = level.time + 15;
					break;
				case 8:
					ent->wait = level.time + 10;
					break;
				default:
					ent->wait = level.time + PROX_TIME_TO_LIVE;
					break;
			}

		ent->think = prox_seek;
		ent->nextthink = level.time + 0.2;
}

//===============
//===============
void prox_land (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*field;
	vec3_t	dir;
	vec3_t	forward, right, up;
	int		makeslave = 0;
	int		movetype = MOVETYPE_NONE;
	int		stick_ok = 0;
	vec3_t	land_point;

	// must turn off owner so owner can shoot it and set it off
	// moved to prox_open so owner can get away from it if fired at pointblank range into
	// wall

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (plane->normal)
	{
		VectorMA (ent->s.origin, -10.0, plane->normal, land_point);
		if (gi.pointcontents (land_point) & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			Prox_Explode (ent);
			return;
		}
	}

        if ((other->svflags & SVF_MONSTER) || other->client || (other->takedamage))
	{
		if(other != ent->teammaster)
			Prox_Explode(ent);

		return;
	}

	vectoangles2 (plane->normal, dir);
	AngleVectors (dir, forward, right, up);

	if (gi.pointcontents (ent->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		Prox_Explode (ent);
		return;
	}

	field = G_Spawn();

	VectorCopy (ent->s.origin, field->s.origin);
	VectorClear(field->velocity);
	VectorClear(field->avelocity);
	VectorSet(field->mins, -PROX_BOUND_SIZE, -PROX_BOUND_SIZE, -PROX_BOUND_SIZE);
	VectorSet(field->maxs, PROX_BOUND_SIZE, PROX_BOUND_SIZE, PROX_BOUND_SIZE);
	field->movetype = MOVETYPE_NONE;
	field->solid = SOLID_TRIGGER;
	field->owner = ent;
	field->classname = "prox_field";
	field->teammaster = ent;
        field->think = Prox_Field_Think;
        field->nextthink = level.time + FRAMETIME;

	gi.linkentity (field);

	// rotate to vertical
	dir[PITCH] = dir[PITCH] + 90;
	VectorCopy (dir, ent->s.angles);
	ent->takedamage = DAMAGE_AIM;
	ent->die = prox_die;
	ent->health = PROX_HEALTH;
        ent->nextthink = level.time + 1;
	ent->think = prox_open;
	ent->touch = NULL;
	ent->solid = SOLID_BBOX;
	// record who we're attached to
//	ent->teammaster = other;
        ent->teamchain = field;

        Stick(ent, other);

	gi.linkentity(ent);
}

//===============
//===============
void fire_proxgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage_multiplier, int speed)
{
	edict_t	*prox;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("start %s    aim %s   speed %d\n", vtos(start), vtos(aimdir), speed);
	prox = G_Spawn();
	VectorCopy (start, prox->s.origin);
	VectorScale (aimdir, speed, prox->velocity);
	VectorMA (prox->velocity, 200 + crandom() * 10.0, up, prox->velocity);
	VectorMA (prox->velocity, crandom() * 10.0, right, prox->velocity);
	VectorCopy (dir, prox->s.angles);
	prox->s.angles[PITCH]-=90;
	prox->movetype = MOVETYPE_BOUNCE;
	prox->solid = SOLID_BBOX; 
	prox->s.effects |= EF_GRENADE;
	prox->clipmask = MASK_SHOT|CONTENTS_LAVA|CONTENTS_SLIME;
        prox->s.renderfx |= RF_IR_VISIBLE;

	//FIXME - this needs to be bigger.  Has other effects, though.  Maybe have to change origin to compensate
	// so it sinks in correctly.  Also in lavacheck, might have to up the distance

//        VectorSet (prox->mins, -6, -6, -6);
//        VectorSet (prox->maxs, 6, 6, 6);
        VectorClear(prox->mins);
        VectorClear(prox->maxs);

        prox->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	prox->owner = self;
	prox->teammaster = self;
	prox->touch = prox_land;
//	prox->nextthink = level.time + PROX_TIME_TO_LIVE;
	prox->think = Prox_Explode;
	prox->dmg = PROX_DAMAGE*damage_multiplier;
	prox->classname = "prox";
        prox->takedamage = DAMAGE_NO;
/*
	prox->svflags |= SVF_DAMAGEABLE;
	prox->flags |= FL_MECHANICAL;
*/
	switch (damage_multiplier)
	{
	case 1:
		prox->nextthink = level.time + PROX_TIME_TO_LIVE;
		break;
	case 2:
		prox->nextthink = level.time + 30;
		break;
	case 4:
		prox->nextthink = level.time + 15;
		break;
	case 8:
		prox->nextthink = level.time + 10;
		break;
	default:
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf ("prox with unknown multiplier %d!\n", damage_multiplier);
		prox->nextthink = level.time + PROX_TIME_TO_LIVE;
		break;
	}

	gi.linkentity (prox);
}

/*
=================
fire_railgrenade
=================
*/

static void RailGrenade_Explode (edict_t *ent)
{
        vec3_t          origin,grenade_angs, forward;
        int                     mod, n;


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
                mod = MOD_RAILGRENADES;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

        mod = MOD_RAILGRENADES;

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

        // Now trace the railgun shots:
        for (n = 0; n < 10; n++)
        {
                grenade_angs[0] = -5 + (crandom() * 2);
                grenade_angs[1] = n*36 + crandom()*2;
                grenade_angs[2] = 0;
                AngleVectors (grenade_angs, forward, NULL, NULL);
                fire_rail (ent->owner, origin, forward, 40, 120);
        }

	G_FreeEdict (ent);
}

static void RailGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
        RailGrenade_Explode (ent);
}

void fire_railgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
        grenade->touch = RailGrenade_Touch;
	grenade->nextthink = level.time + timer;
        grenade->think = RailGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

/*
=================
fire_stickinggrenade
=================
*/

static void StickingGrenade_Explode (edict_t *ent)
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
                mod = MOD_STICKINGGRENADES;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

        mod = MOD_STICKINGGRENADES;

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

static void StickingGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	}
        else
                ent->enemy = other;

        ent->solid = SOLID_NOT;
        Stick(ent, other);
        VectorSet(ent->avelocity, 0, 0, 100);
}

void fire_stickinggrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
        grenade->touch = StickingGrenade_Touch;
	grenade->nextthink = level.time + timer;
        grenade->think = StickingGrenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

/*
=================
fire_buckyball
=================
*/

static void BuckyBall_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;
        vec3_t dir;


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
                mod = MOD_BUCKY;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

        mod = MOD_BUCKY;

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

        ent->enemy = NULL;

        if (ent->count == 5)
                G_FreeEdict(ent);
        else
        {
                //Tornar a posar vel
                VectorCopy(ent->velocity, dir);
                VectorNormalize (dir);
                VectorScale(dir, ent->health, ent->velocity);
        }

        ent->count++;
}

static void BuckyBall_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

#if 0
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
#endif
        if (other->takedamage)
                ent->enemy = other;

        BuckyBall_Explode (ent);
}

void BuckyBall_Think (edict_t *ent);

void BuckyBall_OnGroundThink (edict_t *ent)
{
        //Check if onground. If so, explode
        if (ent->groundentity || !ent->velocity[2])
        {
                BuckyBall_Explode(ent);
                ent->nextthink = level.time + 5*FRAMETIME;
        }
        else
        {
                ent->think = BuckyBall_Think;
                ent->nextthink = level.time + FRAMETIME;
        }
}

void BuckyBall_Think (edict_t *ent)
{
        //Check if onground. If so, prepare explode
        if (ent->groundentity || !ent->velocity[2])
        {
                ent->think = BuckyBall_OnGroundThink;
                ent->nextthink = level.time + 5*FRAMETIME;
        }
        else
                ent->nextthink = level.time + FRAMETIME;
}

void fire_buckyball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius)
{
        edict_t *buckyball;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
        AngleVectors (dir, forward, right, up);

        buckyball = G_Spawn();
        VectorCopy (start, buckyball->s.origin);
        VectorScale (aimdir, speed, buckyball->velocity);
        VectorMA (buckyball->velocity, 200 + crandom() * 10.0, up, buckyball->velocity);
        VectorMA (buckyball->velocity, crandom() * 10.0, right, buckyball->velocity);
        VectorSet (buckyball->avelocity, 300, 300, 300);
        buckyball->movetype = MOVETYPE_BOUNCE;
        buckyball->clipmask = MASK_SHOT;
        buckyball->solid = SOLID_BBOX;
        buckyball->s.effects |= EF_GRENADE;
        VectorClear (buckyball->mins);
        VectorClear (buckyball->maxs);
        buckyball->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
        buckyball->owner = self;
        buckyball->touch = BuckyBall_Touch;
        buckyball->nextthink = level.time + FRAMETIME;
        buckyball->think = BuckyBall_Think;
        buckyball->dmg = damage;
        buckyball->dmg_radius = damage_radius;
        buckyball->classname = "buckyball";
        buckyball->takedamage = DAMAGE_NO;
        buckyball->health = 1000;
        buckyball->count = 0;

        gi.linkentity (buckyball);
}

#if 0
// homing
void homing_think (edict_t *ent) 

{
 edict_t *target = NULL;
 edict_t *blip = NULL;
 vec3_t targetdir, blipdir;
  vec_t speed;

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
 //is this the first time we locked in? sound warning for the target
 if (ent->homing_lock == 0)
 {
 gi.sound (target, CHAN_AUTO, gi.soundindex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
 ent->homing_lock = 1;
 }

 }

 ent->nextthink = level.time + .1;
}
/*
=================
fire_homing
=================
*/
void homing_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 120, 120, MOD_HOMING);
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

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_H_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_homing (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
//	rocket->nextthink = level.time + 8000/speed;
//	rocket->think = G_FreeEdict;
          rocket->homing_lock = 0;

            rocket->nextthink = level.time + .1;
            rocket->think = homing_think;
	rocket->dmg = damage;
	rocket->radius_dmg = 120;
	rocket->dmg_radius = 120;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}
#endif

/*
        Trip Bombs
*/
// spawnflags
#define CHECK_BACK_WALL 1

// variables
#define TBOMB_TIMEOUT	180
#define TBOMB_DAMAGE 150
#define TBOMB_RADIUS_DAMAGE 384
#define TBOMB_HEALTH 100
#define TBOMB_SHRAPNEL	5
#define TBOMB_SHRAPNEL_DMG      20

void shrapnel_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// do damage if we can
	if (!other->takedamage)
		return;

	if (VectorCompare(ent->velocity, vec3_origin))
		return;

	T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, 
                plane->normal, TBOMB_SHRAPNEL_DMG, 8, 0, MOD_HG_SPLASH);
	G_FreeEdict(ent);
}

void TripBomb_Explode (edict_t *ent)
{
	vec3_t origin;
	int i = 0;

        T_RadiusDamage(ent, ent->owner ? ent->owner : ent, ent->dmg, ent->enemy, ent->dmg_radius, MOD_HG_SPLASH);

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

	// throw off some debris
	for (i = 0; i < TBOMB_SHRAPNEL; i++)
	{
		edict_t *sh = G_Spawn();
		vec3_t forward, right, up;
		sh->classname = "shrapnel";
		sh->movetype = MOVETYPE_BOUNCE;
		sh->solid = SOLID_BBOX;
		sh->s.effects |= EF_GRENADE;
                sh->s.modelindex = gi.modelindex("models/objects/debris2/tris.md2");
		sh->owner = ent->owner;
		VectorSet (sh->avelocity, 300, 300, 300);
		VectorCopy(ent->s.origin, sh->s.origin);
		AngleVectors (ent->s.angles, forward, right, up);
		VectorScale(forward, 500, forward);
		VectorMA(forward, crandom()*500, right, forward);
		VectorMA(forward, crandom()*500, up, forward);
		VectorCopy(forward, sh->velocity);
		sh->touch = shrapnel_touch;
		sh->think = G_FreeEdict;
		sh->nextthink = level.time + 3.0 + crandom() * 1.5;
	}


	G_FreeEdict(ent);
}

void tripbomb_laser_think (edict_t *self)
{
        vec3_t start, forward;
	vec3_t end;
	trace_t	tr;
	int		count = 8;

	self->nextthink = level.time + FRAMETIME;

	if (level.time > self->timeout)
	{
		// play a sound
		//gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_trig.wav"), 1, ATTN_NORM, 0);
		
		// blow up
		self->chain->think = TripBomb_Explode;
		self->chain->nextthink = level.time + FRAMETIME;
		G_FreeEdict(self);
		return;
	}

	self->svflags &= ~SVF_NOCLIENT;
	VectorCopy (self->s.origin, start);

        AngleVectors(self->chain->s.angles, forward, NULL, NULL);

        VectorMA (start, 2048, forward, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);

	if (!tr.ent)
		return;

//        VectorSubtract(tr.endpos, self->move_origin, delta);
	if (VectorCompare(self->s.origin, self->move_origin))
	{
		// we haven't done anything yet
		if (self->spawnflags & 0x80000000)
		{
			self->spawnflags &= ~0x80000000;
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_LASER_SPARKS);
			gi.WriteByte (count);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			gi.WriteByte (self->s.skinnum);
			gi.multicast (tr.endpos, MULTICAST_PVS);
		}
	}
//        else if (VectorLength(delta) > 1.0)
        else if (tr.ent)
	{
                if (tr.ent->takedamage)
                {
                        // play a sound
                        //gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_trig.wav"), 1, ATTN_NORM, 0);
		
                        // blow up
                        self->chain->think = TripBomb_Explode;
                        self->chain->nextthink = level.time + FRAMETIME;
                        G_FreeEdict(self);
                        return;
                }
        }
        VectorCopy(tr.endpos, self->move_origin);
	VectorCopy(self->move_origin, self->s.old_origin);
}

void tripbomb_laser_on (edict_t *self)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->think = tripbomb_laser_think;

	// play a sound
        //gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_arm.wav"), 1, ATTN_NORM, 0);
	tripbomb_laser_think(self);
	//gi.positioned_sound(self->s.old_origin, self, CHAN_AUTO, gi.soundindex("weapons/ired/las_tink.wav"), 1, ATTN_NORM, 0);
}

void create_tripbomb_laser(edict_t *bomb)
{
	// create the laser
	edict_t *laser = G_Spawn();
	bomb->chain = laser;
	laser->classname = "laser trip bomb laser";
	VectorCopy(bomb->s.origin, laser->s.origin);
	VectorCopy(bomb->s.origin, laser->move_origin);
	VectorCopy(bomb->s.angles, laser->s.angles);
	G_SetMovedir (laser->s.angles, laser->movedir);
	laser->owner = bomb;
        laser->s.skinnum = 0xf2f2f0f0; // <- faint purple  0xf3f3f1f1 <-blue  red-> 0xf2f2f0f0;
	laser->s.frame = 2;
	laser->movetype = MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1;
	laser->chain = bomb;
	laser->spawnflags |= 0x80000001;
	laser->think = tripbomb_laser_on;
	laser->nextthink = level.time + FRAMETIME;
	laser->svflags |= SVF_NOCLIENT;
	laser->timeout = level.time + TBOMB_TIMEOUT;
        Stick(laser, bomb);
	gi.linkentity (laser);
}

void use_tripbomb(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->chain)
	{
		// we already have a laser, remove it
		G_FreeEdict(self->chain);
		self->chain = NULL;
	}
	else
		// create the laser
		create_tripbomb_laser(self);
}

#if 0
void turnOffGlow(edict_t *self)
{
	self->s.effects &= ~EF_COLOR_SHELL;
	self->s.renderfx &= ~RF_SHELL_GREEN;
	self->think = NULL;
	self->nextthink = 0;
}
#endif
static void tripbomb_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	// play the green glow sound
	//gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_glow.wav"), 1, ATTN_NORM, 0);

//Die!
                self->think = TripBomb_Explode;
                self->nextthink = level.time + FRAMETIME;
                if (self->chain) G_FreeEdict(self->chain);
}

static void tripbomb_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	tripbomb_pain(self, inflictor, 0, 0);
}

void tripbomb_think(edict_t *self)
{
	if (self->chain == NULL)
	{
		// check whether we need to create the laser
		if (self->timeout < level.time)
		{
			create_tripbomb_laser(self);
		}
	}

  	self->nextthink = level.time + FRAMETIME;
}

void setupBomb(edict_t *bomb, char *classname, float damage, float damage_radius)
{
	bomb->classname = classname;
//        VectorSet(bomb->mins, -8, -8, -8);
//        VectorSet(bomb->maxs, 8, 8, 8);
        VectorClear(bomb->mins);
        VectorClear(bomb->maxs);
	bomb->solid = SOLID_BBOX;
	bomb->movetype = MOVETYPE_NONE;
        bomb->s.modelindex = gi.modelindex("models/objects/grenade2/tris.md2");
	bomb->radius_dmg = damage;
	bomb->dmg = damage;
	bomb->dmg_radius = damage_radius;
        bomb->health = 30;
        bomb->takedamage = DAMAGE_YES;
//        bomb->pain = tripbomb_pain;
        bomb->die = tripbomb_die;
}

void removeOldest()
{
	edict_t *oldestEnt = NULL;
	edict_t *e = NULL;
	int count = 0;

	while(1)
	{
                e = G_Find(e, FOFS(classname), "tripbomb");
		if (e == NULL) // no more
			break;

		count++;

		if (oldestEnt == NULL ||
			e->timestamp < oldestEnt->timestamp)
		{
			oldestEnt = e;
		}
	}

	// do we have too many?
	if (count > TBOMB_MAX_EXIST && oldestEnt != NULL)
	{
		// get this tbomb to explode
		oldestEnt->think = TripBomb_Explode;
		oldestEnt->nextthink = level.time + FRAMETIME;
		G_FreeEdict(oldestEnt->chain);
	}
}

qboolean fire_tripbomb(edict_t *self, vec3_t start, vec3_t dir, float timer, float damage, float damage_radius, qboolean quad)
{
	// trace a line
	trace_t tr;
	vec3_t endPos;
	vec3_t _dir;
	edict_t *bomb = NULL;
	edict_t *laser = NULL;

	VectorScale(dir, 64, _dir);
	VectorAdd(start, _dir, endPos);

	// trace ahead, looking for a wall
	tr = gi.trace(start, NULL, NULL, endPos, self, MASK_SHOT);
	if (tr.fraction == 1.0)
	{
		// not close enough
		//gi.cprintf(self, PRINT_HIGH, "Not close enough to a wall");
		return false;
	}

        if (tr.ent)
        if (tr.ent->client || tr.ent->takedamage)
	{
		//gi.cprintf(self, PRINT_HIGH, "Hit something other than a wall");
		return false;
	}

	// create the bomb
	bomb = G_Spawn();
	//VectorCopy(tr.endpos, bomb->s.origin);
	VectorMA(tr.endpos, 3, tr.plane.normal, bomb->s.origin);
	vectoangles(tr.plane.normal, bomb->s.angles);
	bomb->owner = self;
        setupBomb(bomb, "tripbomb", damage, damage_radius);
	gi.linkentity(bomb);

	bomb->timestamp = level.time;
	bomb->timeout = level.time + timer;
	bomb->nextthink = level.time + FRAMETIME;
	bomb->think = tripbomb_think;
        Stick(bomb, tr.ent);

	// remove the oldest trip bomb
        removeOldest();

	// play a sound
        gi.sound (self, CHAN_AUTO, gi.soundindex ("lsrbmbpt.wav"), 1, ATTN_NORM, 0);
	return true;
}

/*
        Trip Bombs
*/
// spawnflags

// variables


void LaserTripBomb_Explode (edict_t *ent)
{
	vec3_t origin;
	int i = 0;

        T_RadiusDamage(ent, ent->owner ? ent->owner : ent, ent->dmg, ent->enemy, ent->dmg_radius, MOD_HG_SPLASH);

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

	G_FreeEdict(ent);
}

void lasertripbomb_laser_think (edict_t *self)
{
        vec3_t start, forward;
	vec3_t end;
	trace_t	tr;
	int		count = 8;
        edict_t *ignore=NULL;

	self->nextthink = level.time + FRAMETIME;

	if (level.time > self->timeout)
	{
		// play a sound
		//gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_trig.wav"), 1, ATTN_NORM, 0);
		
		// blow up
		self->chain->think = TripBomb_Explode;
		self->chain->nextthink = level.time + FRAMETIME;
		G_FreeEdict(self);
		return;
	}

	self->svflags &= ~SVF_NOCLIENT;
	VectorCopy (self->s.origin, start);

        AngleVectors(self->chain->s.angles, forward, NULL, NULL);

        VectorMA (start, 2048, forward, end);

	while(1)
	{
		tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		if (!tr.ent)
			break;

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER))
			T_Damage (tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_TARGET_LASER);

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		{
//                        if (self->spawnflags & 0x80000000)
//                        {
//                                self->spawnflags &= ~0x80000000;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (count);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
//                        }
			break;
		}

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}


        VectorCopy(tr.endpos, self->move_origin);
	VectorCopy(self->move_origin, self->s.old_origin);
}

void lasertripbomb_laser_on (edict_t *self)
{
	self->svflags &= ~SVF_NOCLIENT;
        self->think = lasertripbomb_laser_think;

	// play a sound
        //gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_arm.wav"), 1, ATTN_NORM, 0);
        lasertripbomb_laser_think(self);
	//gi.positioned_sound(self->s.old_origin, self, CHAN_AUTO, gi.soundindex("weapons/ired/las_tink.wav"), 1, ATTN_NORM, 0);
}

void create_lasertripbomb_laser(edict_t *bomb)
{
	// create the laser
	edict_t *laser = G_Spawn();
	bomb->chain = laser;
	laser->classname = "laser trip bomb laser";
	VectorCopy(bomb->s.origin, laser->s.origin);
	VectorCopy(bomb->s.origin, laser->move_origin);
	VectorCopy(bomb->s.angles, laser->s.angles);
	G_SetMovedir (laser->s.angles, laser->movedir);
	laser->owner = bomb;
        laser->s.skinnum = 0xf3f3f1f1; // <- faint purple  0xf3f3f1f1 <-blue  red-> 0xf2f2f0f0;
	laser->s.frame = 2;
	laser->movetype = MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1;
	laser->chain = bomb;
	laser->spawnflags |= 0x80000001;
        laser->think = lasertripbomb_laser_on;
	laser->nextthink = level.time + FRAMETIME;
	laser->svflags |= SVF_NOCLIENT;
	laser->timeout = level.time + TBOMB_TIMEOUT;
        laser->activator = bomb->owner;
        laser->dmg =  40;
        Stick(laser, bomb);
	gi.linkentity (laser);
}

void use_lasertripbomb(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->chain)
	{
		// we already have a laser, remove it
		G_FreeEdict(self->chain);
		self->chain = NULL;
	}
	else
		// create the laser
                create_lasertripbomb_laser(self);
}

void lasertripbomb_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	// play the green glow sound
	//gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/ired/las_glow.wav"), 1, ATTN_NORM, 0);

//Die!
                self->think = LaserTripBomb_Explode;
                self->nextthink = level.time + FRAMETIME;
                if (self->chain) G_FreeEdict(self->chain);
}

void lasertripbomb_think(edict_t *self)
{
	if (self->chain == NULL)
	{
		// check whether we need to create the laser
		if (self->timeout < level.time)
		{
                        create_lasertripbomb_laser(self);
		}
	}

  	self->nextthink = level.time + FRAMETIME;
}


qboolean fire_lasertripbomb(edict_t *self, vec3_t start, vec3_t dir, float timer, float damage, float damage_radius, qboolean quad)
{
	// trace a line
	trace_t tr;
	vec3_t endPos;
	vec3_t _dir;
	edict_t *bomb = NULL;
	edict_t *laser = NULL;

	VectorScale(dir, 64, _dir);
	VectorAdd(start, _dir, endPos);

	// trace ahead, looking for a wall
	tr = gi.trace(start, NULL, NULL, endPos, self, MASK_SHOT);
	if (tr.fraction == 1.0)
	{
		// not close enough
		//gi.cprintf(self, PRINT_HIGH, "Not close enough to a wall");
		return false;
	}

        if (tr.ent)
        if (tr.ent->client || tr.ent->takedamage)
	{
		//gi.cprintf(self, PRINT_HIGH, "Hit something other than a wall");
		return false;
	}

	// create the bomb
	bomb = G_Spawn();
	//VectorCopy(tr.endpos, bomb->s.origin);
	VectorMA(tr.endpos, 3, tr.plane.normal, bomb->s.origin);
	vectoangles(tr.plane.normal, bomb->s.angles);
	bomb->owner = self;
        setupBomb(bomb, "tripbomb", damage, damage_radius);
	gi.linkentity(bomb);
	bomb->timestamp = level.time;
	bomb->timeout = level.time + timer;
	bomb->nextthink = level.time + FRAMETIME;
        bomb->think = lasertripbomb_think;
        Stick(bomb, tr.ent);

	// remove the oldest trip bomb
        removeOldest();

	// play a sound
        gi.sound (self, CHAN_AUTO, gi.soundindex ("lsrbmbpt.wav"), 1, ATTN_NORM, 0);
	return true;
}

/*
=================
fire_energyvortex
=================
*/
void energyvortex_explode (edict_t *self)
{
        T_RadiusDamage(self, self->enemy, 200, NULL, 100, MOD_VORTEX);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_ROCKET_EXPLOSION);
        gi.WritePosition (self->s.origin);
        gi.multicast (self->s.origin, MULTICAST_PHS);

        G_FreeEdict(self);
}

void energyvortex_active_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}


	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
        {
                T_Damage (other, self, self->enemy, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_VORTEX);

                if (self->enemy->client)
                        PlayerNoise(self->enemy, self->s.origin, PNOISE_IMPACT);

                gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
        }
}


void energyvortex_think (edict_t *self)
{
        vec3_t dir;
        vec3_t delta;
        float percent, distance;
        float max_mass = 800; //max mass that can be moved...
        float actual_mass;
        edict_t *ent;
        int strenght=110;
        float inRange=760;

        if (!(self->s.effects & EF_TRAP))
        {
                if (self->groundentity)
                {
                        while (self->s.angles[0] >= 360)
                                self->s.angles[0] -= 360;

                        while (self->s.angles[0] <= -360)
                                self->s.angles[0] += 360;

                        if (self->s.angles[0] > 0)
                        {
                                self->s.angles[0] -= 20;
                                if (self->s.angles[0] < 0) self->s.angles[0] = 0;
                        }
                        else if (self->s.angles[0] < 0)
                        {
                                self->s.angles[0] += 20;
                                if (self->s.angles[0] > 0) self->s.angles[0] = 0;
                        }

                        while (self->s.angles[2] >= 360)
                                self->s.angles[2] -= 360;

                        while (self->s.angles[2] <= -360)
                                self->s.angles[2] += 360;

                        if (self->s.angles[2] > 0)
                        {
                                self->s.angles[2] -= 20;
                                if (self->s.angles[2] < 0) self->s.angles[2] = 0;
                        }
                        else if (self->s.angles[2] < 0)
                        {
                                self->s.angles[2] += 20;
                                if (self->s.angles[2] > 0) self->s.angles[2] = 0;
                        }
                }
                else
                {
                        VectorCopy(self->velocity, dir);
                        VectorNormalize (dir);
                        vectoangles(dir, self->s.angles); //Wyrm: adjust angle!!
                        self->s.angles[0] -= 90;
                }

                if (self->timestamp < level.time)
                {
                        self->s.effects = EF_TRAP|EF_ROTATE;
                        self->touch = energyvortex_active_touch;
                        self->timestamp = level.time + 3.2;
                        self->owner = NULL;
                        //Wyrm: set a bigger box...
                        VectorSet(self->mins, -16, -16, 0);
                        VectorSet(self->maxs, 16, 16, 32);
                        self->movetype = MOVETYPE_NONE;
                        gi.sound(self, CHAN_AUTO, gi.soundindex("grav_new.wav"), 1, ATTN_NORM, 0);
                }
        }
        else
        {
                //pull stuff
                ent=NULL;
                while ((ent = horn_findradius(ent, self->s.origin, inRange)) != NULL)
                {
             
                        if (visible(self,ent) &&
                           //horn_infront(self, ent)&&
                           ent != self)
                        {
                                if ((ent->svflags & SVF_DEADMONSTER)&&(ent->movetype == MOVETYPE_NONE))
                                {
                                        ent->movetype = MOVETYPE_TOSS;
                                        ent->s.origin[2] += 10;
                                }

                                if (ent->movetype != MOVETYPE_NONE &&
                                    ent->movetype != MOVETYPE_NOCLIP &&
                                    ent->movetype != MOVETYPE_PUSH &&
                                    ent->movetype != MOVETYPE_STOP &&
                                    strcmp(ent->classname, "bolt") &&
                                    strcmp(ent->classname, "item_flag_team1") &&
                                    strcmp(ent->classname, "item_flag_team2")
                                    )
                                {

                                        VectorSubtract(ent->s.origin, self->s.origin, delta);
                                        distance = VectorLength(delta);
                                        if (distance <= 32)
                                                self->touch(self, ent, NULL, NULL);

                                        percent = (inRange - distance) / inRange;


                                        if (ent->groundentity)
                                        {
                                                ent->groundentity = NULL;
                                                ent->s.origin[2]++;
                                                if (!ent->client)
                                                {
                                                        if (delta[2] > 0)
                                                                delta[2] = delta[2] / -2;
                                                        VectorScale (delta, 0.7, delta);
                                                        if (delta[2] > -50)
                                                                delta[2] = -50;
                                                }
                                        }

                                        VectorNormalize(delta);

                                        if (ent->mass < 100) actual_mass = 100;
                                        else actual_mass = ent->mass;

                                        VectorScale(delta, percent * strenght * (max_mass / actual_mass), delta);

                                        VectorMA(ent->velocity, -1, delta, ent->velocity);

                                }
                        }
                }

                if (self->timestamp < level.time)
                {
                        energyvortex_explode (self);
                        return;
                }
        }

        self->think = energyvortex_think;
	self->nextthink = level.time + FRAMETIME;
}

void energyvortex_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}
}


void fire_energyvortex (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
        edict_t *vortex;

        vortex = G_Spawn();
        VectorCopy (start, vortex->s.origin);
        VectorCopy (dir, vortex->movedir);
        vectoangles (dir, vortex->s.angles);
        VectorScale (dir, speed, vortex->velocity);
        vortex->movetype = MOVETYPE_TOSS;
        vortex->clipmask = MASK_SHOT;
//        vortex->solid = SOLID_TRIGGER;
        vortex->solid = SOLID_BBOX;
        vortex->s.effects |= EF_BLASTER;
//        VectorClear (vortex->mins);
//        VectorClear (vortex->maxs);
        VectorClear(vortex->mins);
        VectorClear(vortex->maxs);

//        VectorSet(vortex->mins, -16, -16, -8);
//        VectorSet(vortex->maxs, 16, 16, 24);

        vortex->s.modelindex = gi.modelindex ("models/items/keys/pyramid/tris.md2");
        vortex->enemy = vortex->owner = self;
        vortex->touch = energyvortex_touch;
        vortex->nextthink = level.time + 8000/speed;
        vortex->think = G_FreeEdict;
        vortex->radius_dmg = damage;
        vortex->dmg_radius = damage_radius;
        vortex->classname = "vortex blast";
//        vortex->think = energyvortex_think;
//        vortex->nextthink = level.time + FRAMETIME;
        vortex->timestamp = level.time + 2;

        vortex->takedamage = DAMAGE_NO;

        energyvortex_think(vortex);

        vortex->teammaster = vortex;
        vortex->teamchain = NULL;

	if (self->client)
                check_dodge (self, vortex->s.origin, dir, speed);

        gi.linkentity (vortex);

}


void fire_vacuummaker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
        vec3_t delta;
        float percent, distance;
        float max_mass = 800; //max mass that can be moved...
        float actual_mass;
        edict_t *ent;
        int strenght=100;
        float inRange=350;

                //pull stuff
                ent=NULL;
                while ((ent = horn_findradius(ent, start, inRange)) != NULL)
                {
             
                        if (visible(self,ent) &&
                           horn_infront(self, ent)&&
                           ent != self)
                        {
                                if ((ent->svflags & SVF_DEADMONSTER)&&(ent->movetype == MOVETYPE_NONE))
                                {
                                        ent->movetype = MOVETYPE_TOSS;
                                        ent->s.origin[2] += 10;
                                }

                                if (ent->movetype != MOVETYPE_NONE &&
                                    ent->movetype != MOVETYPE_NOCLIP &&
                                    ent->movetype != MOVETYPE_PUSH &&
                                    ent->movetype != MOVETYPE_STOP &&
                                    strcmp(ent->classname, "bolt") &&
                                    strcmp(ent->classname, "item_flag_team1") &&
                                    strcmp(ent->classname, "item_flag_team2")
                                    )
                                {

                                        VectorSubtract(ent->s.origin, start, delta);
                                        distance = VectorLength(delta);
                                        if (distance <= 48)
                                        {
                                                if (ent->takedamage)
                                                        T_Damage (ent, self, self, self->velocity, start, dir, 50, 0, 0, MOD_VACUUM);
                                                if (ent->touch)
                                                        ent->touch(ent, self, NULL, NULL);
                                        }

                                        percent = (inRange - distance) / inRange;


                                        if (ent->groundentity)
                                        {
                                                ent->groundentity = NULL;
                                                ent->s.origin[2]++;
                                                /*
												if (!ent->client)
                                                {
                                                        if (delta[2] > 0)
                                                                delta[2] = delta[2] / -2;
                                                        VectorScale (delta, 0.7, delta);
                                                        if (delta[2] > -80)
                                                                delta[2] = -80;
                                                }
												*/
										}

                                        VectorNormalize(delta);

                                        if (ent->mass < 100) actual_mass = 100;
                                        else actual_mass = ent->mass;

                                        VectorScale(delta, percent * strenght * (max_mass / actual_mass), delta);

                                        VectorMA(ent->velocity, -1, delta, ent->velocity);

                                }
                        }
                }

	gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_SPARKS);
//      gi.WriteByte ();
        gi.WritePosition (start);
        gi.WriteDir (dir);
        gi.multicast (start, MULTICAST_PVS);
}

static void ShrapnelThink(edict_t *ent)
{
        ent->s.effects = 0;
        VectorAdd(ent->movedir, ent->velocity, ent->velocity);
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 3.0 + crandom() * 1.5;
}

void fire_flakcannon (edict_t *self, vec3_t start, vec3_t aimdir, int ammount, int speed)
{
        int i;
        edict_t *sh;
        vec3_t forward, right, up, dir;

        vectoangles(aimdir, dir);
        AngleVectors (dir, forward, right, up);

	// throw off some debris
        for (i = 0; i < ammount; i++)
	{
                sh = G_Spawn();
		sh->classname = "shrapnel";
		sh->movetype = MOVETYPE_BOUNCE;
		sh->solid = SOLID_BBOX;
                sh->clipmask = MASK_SHOT;
                VectorClear(sh->mins);
                VectorClear(sh->maxs);
//                sh->s.effects |= EF_GRENADE;
                sh->s.modelindex = gi.modelindex("models/objects/debris2/tris.md2");
                sh->owner = self;
                VectorSet (sh->avelocity, 300*crandom(), 300*crandom(), 300*crandom());
                VectorSet (sh->s.angles, 360*crandom(), 360*crandom(), 360*crandom());
                VectorCopy(start, sh->s.origin);
                VectorScale(forward, speed, dir);
                VectorMA(dir, (speed/6), up, dir);
                VectorCopy(dir, sh->velocity);
                VectorClear(dir);
                VectorMA(dir,crandom()*100, up, dir);
                VectorMA(dir, crandom()*80, right, dir);
                VectorCopy(dir, sh->movedir);
		sh->touch = shrapnel_touch;
                sh->think = ShrapnelThink;
                sh->nextthink = level.time + 2*FRAMETIME;
//                if (i == ammount - 1)
//                        sh->s.effects |= EF_ROCKET;
                        
                gi.linkentity(sh);
	}
}


void drainer_dowork(edict_t *ent)
{
        if (ent->dmg > 0)
        {
                if (ent->enemy->health > 0)
                {
                        vec3_t dir;
                
                        AngleVectors(ent->s.angles, dir, NULL, NULL);
                        VectorScale(dir, -1, dir);
                        T_Damage(ent->enemy, ent, ent->owner, ent->velocity, ent->s.origin, dir, 2, 0, DAMAGE_NO_ARMOR, MOD_DRAINER);
                        gi.sound (ent, CHAN_AUTO, gi.soundindex("drainer/drain.wav"), 0.9, ATTN_NORM, 0);
                }
                else
                        ent->movetype = MOVETYPE_TOSS;

                ent->dmg -= 2;
                ent->nextthink = level.time + FRAMETIME;
        }
        else
                G_FreeEdict(ent);
}

void drainer_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

        if ((other->svflags & SVF_MONSTER)||(other->client))
        {
                ent->solid = SOLID_NOT;
                Stick (ent, other);
                ent->enemy = other;
                ent->think = drainer_dowork;
                ent->nextthink = level.time + FRAMETIME;
        }
        else
        {
                if (other->takedamage)
                        T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_HIT);
                else
                {
                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_GUNSHOT);
                        gi.WritePosition (ent->s.origin);
                        gi.WriteDir (plane->normal);
                        gi.multicast (ent->s.origin, MULTICAST_PVS);
                }

                G_FreeEdict(ent);
        }                
}

void fire_blooddrainer (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
        edict_t *drainer;

	VectorNormalize (dir);

        drainer = G_Spawn ();
        VectorCopy (start, drainer->s.origin);
        VectorCopy (start, drainer->s.old_origin);
        vectoangles (dir, drainer->s.angles);
        VectorScale (dir, 800, drainer->velocity);

        drainer->movetype = MOVETYPE_FLYMISSILE;
        drainer->clipmask = MASK_SHOT;
        drainer->solid = SOLID_BBOX;

        VectorClear (drainer->mins);
        VectorClear (drainer->maxs);
        drainer->s.modelindex = gi.modelindex ("models/drainer/tris.md2");
        drainer->owner = self;
        drainer->touch = drainer_touch;
        drainer->nextthink = level.time + 3;
        drainer->think = G_FreeEdict;
        drainer->dmg = damage;
        gi.linkentity (drainer);

	if (self->client)
                check_dodge (self, drainer->s.origin, dir, 800);

}


void fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	trace_t		tr;
        vec3_t end;
        vec3_t offset;
        edict_t *ent;

        //Wyrm: check for viewent
        if (self->client && (self->client->chasetoggle == 1 || self->client->missile)&&(self->client->oldplayer))
        {
                ent = self->client->oldplayer;
                VectorClear(offset);
        }
        else
        {
                ent = self;
                VectorSubtract(start, ent->s.origin, offset);
        }

        VectorMA (start, 600, dir, end);

        tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT| MASK_WATER);

        if (tr.fraction < 1.0) //Ha tocat algo
        {
                if (tr.contents & MASK_WATER)
                {
                        T_WaterRadiusDamage (self, self, 35, NULL, 80, MOD_LIGHTNING);
                }
                else if ((tr.ent != NULL)&&(tr.ent->takedamage)) //Ha tocat una entitat que es pot ferir
                {
                        //Li fem mal!
                        T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, damage, 0, DAMAGE_ENERGY, MOD_LIGHTNING);
                }
                else
                {
                        //Fem sparks
                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_SPARKS);
                        gi.WritePosition (tr.endpos);
                        gi.WriteDir (tr.plane.normal);
                        gi.multicast (self->s.origin, MULTICAST_PVS);
                }
        }

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GRAPPLE_CABLE);
        gi.WriteShort (ent - g_edicts);
        gi.WritePosition (start);
        gi.WritePosition (tr.endpos);
	gi.WritePosition (offset);

        gi.multicast (ent->s.origin, MULTICAST_PVS);
}

/*
        fire_nail
*/

// RAFAEL
void spike_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

        if (self->enemy->client)
                        PlayerNoise (self->enemy, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, self->count);

	}
	else
	{
                if (plane)
                {
                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_GUNSHOT);
                //      gi.WriteByte (damage);
                        gi.WritePosition (self->s.origin);
                        gi.WriteDir (plane->normal);
                        gi.multicast (self->s.origin, MULTICAST_PVS);
                }
	}

	G_FreeEdict (self);
}


// RAFAEL
void fire_nail (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int mod)
{
        edict_t *spike;
	trace_t tr;

	VectorNormalize (dir);

        spike = G_Spawn ();
        VectorCopy (start, spike->s.origin);
        VectorCopy (start, spike->s.old_origin);
        vectoangles (dir, spike->s.angles);
        VectorScale (dir, speed, spike->velocity);

        spike->movetype = MOVETYPE_FLYMISSILE;
        spike->clipmask = MASK_SHOT;
        spike->solid = SOLID_BBOX;

        VectorClear (spike->mins);
        VectorClear (spike->maxs);
        spike->s.modelindex = gi.modelindex ("models/nail/tris.md2");
        spike->enemy = spike->owner = self;
        spike->touch = spike_touch;
        spike->nextthink = level.time + 6;
        spike->think = G_FreeEdict;
        spike->dmg = damage;
        spike->count = mod;

        gi.linkentity (spike);


	if (self->client)
                check_dodge (self, spike->s.origin, dir, speed);

        tr = gi.trace (self->s.origin, NULL, NULL, spike->s.origin, spike, MASK_SHOT);
        if (tr.fraction < 1.0)
	{
                VectorMA (spike->s.origin, -10, dir, spike->s.origin);
                spike->touch (spike, tr.ent, NULL, NULL);
	}

}
/*
=========
fire_nuke
=========
*/
void nuke_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        vec3_t  end, aimdir;
        vec3_t  cloud = {10, 100, 100};
        vec3_t  timer = {0.2, 0.4, 1};
        vec3_t  fire_damage = { 6, 4, 25 };
        vec3_t  radii = { 120, 500, 2000};
        int radius_damage = 500;
        int damage;

//        if (deathmatch->value)
//                damage = 400;
//        else
                damage = 1000;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

        VectorCopy(ent->velocity, aimdir);
        VectorNormalize(aimdir);

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

        if (other->takedamage)
        {
                if (other->client)
                {       /* Blind target. */
                        other->client->flash_alpha = 1;
                        other->client->flash_fade = 0.05;
                }
                PBM_Ignite (other, ent->owner, ent->s.origin);
                T_Damage (other, ent->owner, ent->owner, aimdir, ent->s.origin, ent->velocity, damage, damage, DAMAGE_ENERGY, MOD_NUKE);
        }

        VectorMA (ent->s.origin, -4, aimdir, end);
        PBM_FlashExplosion (ent->owner, ent->s.origin, end, radius_damage, radii, NULL, MOD_NUKE);
        PBM_FlameCloud (ent->owner, ent->s.origin, cloud, timer, true, fire_damage, fire_damage, 0, 100);

	G_FreeEdict (ent);
}

void fire_nuke (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
        edict_t *nuke;

        nuke = G_Spawn();
        VectorCopy (start, nuke->s.origin);
        VectorCopy (dir, nuke->movedir);
        vectoangles (dir, nuke->s.angles);
        VectorScale (dir, speed, nuke->velocity);
        nuke->movetype = MOVETYPE_FLYMISSILE;

        nuke->clipmask = MASK_SHOT;
        nuke->solid = SOLID_BBOX;
        nuke->s.effects = EF_ROCKET;
        VectorClear (nuke->mins);
        VectorClear (nuke->maxs);
        nuke->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
        nuke->owner = self;
        nuke->touch = nuke_touch;
        nuke->nextthink = level.time + 8000/speed;
        nuke->think = G_FreeEdict;
        nuke->dmg = damage;
        nuke->radius_dmg = radius_damage;
        nuke->dmg_radius = damage_radius;
        nuke->s.sound = gi.soundindex ("weapons/rockfly.wav");
        nuke->classname = "nuke";

	if (self->client)
                check_dodge (self, nuke->s.origin, dir, speed);

        gi.linkentity (nuke);
}

/*
=========
fire_homing
=========
*/

void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void fire_homing (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
        rocket->movetype = MOVETYPE_HOMING;

	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
        rocket->classname = "homing";

//        if (self->client)
//                check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

/*
=================
fire_perforator
=================
*/
static void Perforator_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;


	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

        mod = MOD_PERFORATOR_SPLASH;

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
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
                gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
                gi.WriteByte (TE_ROCKET_EXPLOSION);

	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void perforator_think (edict_t *self)
{
        vec3_t right, up, dir;
        int frommonster = false;

        if (self->owner && (self->owner->inuse))
                if (!self->owner->client)
                        frommonster = true;

        if ((level.time > self->timeout)||(self->enemy->solid == SOLID_NOT)||(!self->enemy->inuse))
        {
                Perforator_Explode(self);
                return;
        }

        AngleVectors(self->s.angles, NULL, right, up);
        VectorScale(right, rand(), dir);
        VectorMA(dir, rand(), up, dir);
        VectorNormalize(dir);

        //hammerstein rocket does less damage... :P
        if (frommonster)
                T_Damage(self->enemy, self, self->owner, self->velocity, self->s.origin, dir, 1, 0, DAMAGE_NO_KNOCKBACK|DAMAGE_DESTROY_ARMOR|DAMAGE_NO_POWER_ARMOR, MOD_PERFORATOR);
        else
                T_Damage(self->enemy, self, self->owner, self->velocity, self->s.origin, dir, 3, 0, DAMAGE_NO_KNOCKBACK|DAMAGE_DESTROY_ARMOR|DAMAGE_NO_POWER_ARMOR, MOD_PERFORATOR);

        if (!self->count)
        {
                gi.sound(self, CHAN_AUTO, gi.soundindex("smack.wav"), 1, ATTN_NORM, 0);
                self->count++;
        }
        else if (self->count == 1)
                self->count = 0;
        else
                self->count++;

        self->nextthink = level.time + FRAMETIME;
}

static void perforator_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

        if (other->takedamage && (other->deadflag || other->client || (other->svflags & SVF_MONSTER)))
	{
                ent->enemy = other;

                T_Damage(ent->enemy, ent, ent->owner, ent->velocity, ent->s.origin, vec3_origin, 10, 0, 0, MOD_PERFORATOR);

                ent->think = perforator_think;
                ent->count = 0;

                ent->nextthink = level.time + FRAMETIME;
                ent->timeout = level.time + 2;
                ent->solid = SOLID_NOT;
                Stick(ent, other);
                VectorSet(ent->avelocity, 0, 0, 800);
	}
        else
                Perforator_Explode(ent);
}

void fire_perforator (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
        edict_t *perforator;

        perforator = G_Spawn();
        VectorCopy (start, perforator->s.origin);
        VectorCopy (dir, perforator->movedir);
        vectoangles (dir, perforator->s.angles);
        VectorScale (dir, speed, perforator->velocity);
        perforator->movetype = MOVETYPE_FLYMISSILE;

        perforator->clipmask = MASK_SHOT;
        perforator->solid = SOLID_BBOX;
        perforator->s.effects |= EF_ROCKET;
        VectorClear (perforator->mins);
        VectorClear (perforator->maxs);
        perforator->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
        perforator->owner = self;
        perforator->touch = perforator_touch;
        perforator->nextthink = level.time + 8000/speed;
        perforator->think = G_FreeEdict;
        perforator->dmg = damage;
        perforator->radius_dmg = radius_damage;
        perforator->dmg_radius = damage_radius;
        perforator->s.sound = gi.soundindex ("weapons/rockfly.wav");
        perforator->classname = "perforator";

        VectorSet(perforator->avelocity, 0, 0, 800);

	if (self->client)
                check_dodge (self, perforator->s.origin, dir, speed);

        gi.linkentity (perforator);
}

/*
==================
 Automatic Sentry
==================
*/
#define SENTRY_HEALTH 300
#define SENTRY_DAMAGE 6

static void fire_sentry_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
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
                                //damage inflicted by turretgun
                                T_Damage (tr.ent, self, self->owner->enemy, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
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

                                        //Wyrm: Bind a bullethole
                                        create_bullethole(tr.ent, tr.endpos, tr.plane.normal);


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

void fire_sentry_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
        fire_sentry_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}

static void SentryExplode(edict_t *self)
{
	vec3_t		origin;

        if (self->oldenemy)
        {
                G_FreeEdict(self->oldenemy);
                self->oldenemy = NULL;
        }

        self->takedamage = DAMAGE_NO;
        T_RadiusDamage(self, self->enemy, 100, self, 140, MOD_SENTRYEXPLODE );

        VectorMA (self->s.origin, -0.02, self->velocity, origin);
	gi.WriteByte (svc_temp_entity);
        if (self->groundentity)
		gi.WriteByte (TE_GRENADE_EXPLOSION);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
        gi.multicast (self->s.origin, MULTICAST_PVS);

        G_FreeEdict (self);
}

static void sentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        SentryExplode(self);
}

static void sentry_pain (edict_t *self, edict_t *other, float kick, int damage)
{
        //attack the attacker :P
        if (self->oldenemy)
        self->oldenemy->enemy = other;
}

static void Turretgun_Think (edict_t *self)
{
        if ((!self->owner)||(!self->owner->inuse))
        {
                G_FreeEdict(self);
                return;
        }

        //turret on attack position
        self->owner->s.sound = 0;

        VectorCopy(self->owner->s.origin, self->s.origin);
        self->s.origin[2] += 22;

        VectorCopy(self->owner->velocity, self->velocity);

        //Check if we've bullets
        if (self->count)
        {
                //Check for enemies....
                if (!self->enemy)
                {
                        edict_t *target, *best;
                        vec3_t  distance_vec;
                        float bestdistance, distance;

                        //Check for new enemy
                        best = NULL;
                        for (target = g_edicts; target < &g_edicts[globals.num_edicts]; target++)
                        {
                
                                if (!target->inuse)
                                        continue;
                                if (target->solid == SOLID_NOT)
                                        continue;
                                if (target->movetype == MOVETYPE_NOCLIP)
                                        continue;

                                //only kill clients and monsters
                                if ((!target->client) && (!(target->svflags & SVF_MONSTER)))
                                        continue;
                                //Don't attack owner!!
                                if (target == self->owner->enemy)
                                        continue;

                                //Attack only visible ents
                                if (!visible(self, target))
                                        continue;

                                if (target->flags & FL_NOTARGET)
                                        continue;

                                if (target->deadflag)
                                        continue;

                                if (self->owner->enemy->client)
                                {
                                        //Check teams
                                        if ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value)
                                                if (OnSameTeam (self, target))
                                                        continue;                                        
                                }
                                else
                                {
                                        //no player turrets only attack players
                                        if (!target->client)
                                                continue;
                                }

                                //Check distances
                                VectorSubtract(target->s.origin, self->s.origin, distance_vec);
                                distance = VectorLength(distance_vec);
                                if (!best || (distance < bestdistance))
                                {
                                        best = target;
                                        bestdistance = distance;
                                }
                        }

                        if (best)
                        {
                                self->enemy = best;
                                VectorSubtract(best->s.origin, self->s.origin, distance_vec);
                                vectoangles(distance_vec, self->s.angles);
                        }
                        else
                        {
                                //Rotate turret while looking for victims...
                                self->owner->s.sound = gi.soundindex ("weapons/turret/online.wav");
                                self->s.angles[1] += 7;
                                //recover z angle
                                if (self->s.angles[0])
                                {
                                        while (self->s.angles[0] >= 360)
                                                self->s.angles[0] -= 360;
                                        
                                        while (self->s.angles[0] < 0)
                                                self->s.angles[0] += 360;
                        
                                        if (self->s.angles[0] <= 180)
                                        {
                                                self->s.angles[0] -= 3;
                                                if (self->s.angles[0] < 0)
                                                        self->s.angles[0] = 0;
                                        }
                                        else
                                        {
                                                self->s.angles[0] += 3;
                                                if (self->s.angles[0] > 360)
                                                        self->s.angles[0] = 0;
                                        }
                                }
                        }
                }
                else
                {
                        //Check if we can kill it
                        if (visible(self, self->enemy) && (!self->enemy->deadflag) && (self->enemy->inuse) && (!(self->enemy->flags & FL_NOTARGET)) )
                        {
                                vec3_t dir;
                                vec3_t end;

                                //Look at enemy
                                VectorCopy(self->enemy->s.origin, end);
                                end[2] += self->enemy->viewheight;

                                VectorSubtract(end, self->s.origin, dir);
                                vectoangles(dir, self->s.angles);
                        
                                //fire a bullet
                                fire_sentry_bullet (self, self->s.origin, dir, SENTRY_DAMAGE, 0,  DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_SENTRYBULLET);
                                gi.positioned_sound (self->s.origin, self, CHAN_WEAPON, gi.soundindex("weapons/machgf1b.wav"), 1, ATTN_NORM, 0);
                                self->count--;
                        }
                        else //look for a new enemy
                                self->enemy = NULL;
                }
        }
        else
        {
                if (self->style)
                        self->style--;
                else
                {
                        self->owner->think = SentryExplode;
                        self->owner->nextthink = level.time + FRAMETIME;
                        self->think = NULL;
                        return;
                }
        }

        self->think = Turretgun_Think;
        self->nextthink = level.time + FRAMETIME;
}

static void ActiveTurretBaseThink (edict_t *self)
{
        vec3_t distance;

        if (!self->owner)
                return;

        VectorSubtract(self->owner->s.origin, self->s.origin, distance);

        if (VectorLength(distance) > 32)
        {
                self->owner = NULL;
                self->think = NULL;
                return;
        }

        self->nextthink = level.time + FRAMETIME;
}

static void InactiveSentry_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
}

void barrel_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

static void InactiveSentry_Think (edict_t *self)
{
        edict_t *turretgun;
        edict_t *turretbase;

        if (!self->count)
        {
                if (VectorLength(self->velocity))
                        self->nextthink = level.time + FRAMETIME;
                else
                {
                        self->count = 1;
                        self->nextthink = level.time + 1;
                }
                return;
        }


        turretgun = G_Spawn();
        turretbase = G_Spawn();

        //transform into a cool turret :P
        turretbase->s.modelindex = gi.modelindex ("models/objects/t_base/tris.md2");
        turretbase->takedamage = DAMAGE_AIM;
        turretbase->die = sentry_die;
        turretbase->pain = sentry_pain;
        turretbase->health = self->health;
        turretbase->think = ActiveTurretBaseThink;
        turretbase->nextthink = level.time + FRAMETIME;
        turretbase->enemy = self->owner;
        turretbase->owner = self->owner;
        turretbase->oldenemy = turretgun;
        turretbase->solid = SOLID_BBOX;
        VectorCopy(self->s.angles, turretbase->s.angles);
        VectorCopy(self->s.origin, turretbase->s.origin);
        VectorCopy(self->mins, turretbase->mins);
        VectorCopy(self->maxs, turretbase->maxs);
        turretbase->s.origin[2] -= 22;
        turretbase->mins[2] += 22;
        turretbase->maxs[2] += 22;
        turretbase->s.sound = gi.soundindex ("weapons/turret/online.wav");
        turretbase->movetype = MOVETYPE_BOUNCE;
        turretbase->mass = self->mass;
        turretbase->touch = barrel_touch;
        
        turretgun->s.modelindex = gi.modelindex ("models/objects/lturret/tris.md2");
        VectorClear(turretgun->mins);
        VectorClear(turretgun->maxs);
        turretgun->owner = turretbase;
        VectorCopy(turretbase->s.angles, turretgun->s.angles);
        turretgun->movetype = MOVETYPE_FLYMISSILE;
        turretgun->solid = SOLID_NOT;
        VectorCopy(turretbase->s.origin, turretgun->s.origin);
        turretgun->s.origin[2] += 22;
        turretgun->think = Turretgun_Think;
        turretgun->nextthink = level.time + FRAMETIME;
        VectorClear(turretgun->velocity);
        turretgun->enemy = NULL;
        turretgun->count = 300;
        turretgun->style = 100;

        gi.linkentity(turretgun);
        gi.linkentity(turretbase);

        G_FreeEdict(self);

}

void Launch_Sentry (edict_t *ent)
{
        edict_t *sentry;
	vec3_t	offset, start;
        vec3_t  forward, right, up;


        AngleVectors (ent->client->v_angle, forward, right, up);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


        sentry = G_Spawn();
        VectorCopy(start, sentry->s.origin);
        VectorMA(up, 2, forward, sentry->velocity);
        VectorNormalize(sentry->velocity);
        VectorScale(sentry->velocity, 200, sentry->velocity);

        sentry->touch = InactiveSentry_Touch;
        sentry->think = InactiveSentry_Think;
        sentry->nextthink = level.time + FRAMETIME;
        sentry->owner = ent;
        sentry->solid = SOLID_BBOX;
        sentry->s.modelindex = gi.modelindex ("models/objects/ltrthrow/tris.md2");
        sentry->movetype = MOVETYPE_BOUNCE;
        sentry->takedamage = DAMAGE_AIM;
        sentry->die = sentry_die;
        sentry->health = SENTRY_HEALTH;
        sentry->count = 0;

        sentry->mass = 250;

        VectorCopy(forward, offset);
        offset[2] = 0;
        vectoangles (offset, sentry->s.angles);

        VectorSet(sentry->mins, -20, -20, -22);
        VectorSet(sentry->maxs, 20, 20, 16);
        gi.linkentity(sentry);
}

