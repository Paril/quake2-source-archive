#include "g_local.h"

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
        T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, 0, TOD_NO_KNOCKBACK, MOD_HIT);

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
GRIM
The entire lead code has been edited to shit.
For all the simplicity I wanted, it looks kinda complex
(Or at least very messy)
=================
*/
/*
=================
GRIM START OF LEAD CODE
=================
*/

void lead_free (edict_t *self);
/*
=================
LeadExplode - For explosive shells/bullets...
=================
*/
static void LeadExplode (edict_t *ent, edict_t *other, vec3_t dir, vec3_t origin, vec3_t normal, int damage, int mod, int type)
{
        edict_t *owner;
        edict_t *thing;

        if ((ent->client) || (ent->svflags & SVF_MONSTER))
        {
                owner = ent;
                thing = G_Spawn();
                VectorCopy (origin, thing->s.origin);
        }
        else
        {
                thing = ent;
                owner = ent->owner;
        }

        if (owner->client)
                PlayerNoise(owner, origin, PNOISE_IMPACT);

        if (type & TOD_RADIUS)
                damage = 80 + (int)(random() * 30);
        else
                damage = 30 + (int)(random() * 10);

        if (other->takedamage) // Added kick of ent->damage, so it is part pro rocket
                T_Damage (other, thing, ent, dir, origin, normal, damage, damage, 0, (TOD_RADIUS | TOD_EXPLOSIVE), mod);

        if (type & TOD_RADIUS)
        {
                T_RadiusDamage(thing, owner, damage, other, damage, mod);
                gi.WriteByte (svc_temp_entity);
                if (ent->waterlevel)
                        gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
                else
                        gi.WriteByte (TE_ROCKET_EXPLOSION);
                gi.WritePosition (origin);
        }
        else
        {
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_SPARKS);
                gi.WritePosition (origin);
                gi.WriteDir (normal);
        }
        gi.multicast (origin, MULTICAST_PVS);
        // Dirty
        if (ent == thing)
                lead_free (ent);
        else
                G_FreeEdict (thing);
        // Dirty
}

/*
===================================================
                LEAD CODE
===================================================
*/

// Paranoid
/*
================
Ricochet - Paranoid only? Custom ricochet sounds...
================
*/
/* Dirty
static void Ricochet (vec3_t point, int count)
{
        static int      i;
        int     r;

        r = rand()%(8*count);

        if (r > 1)
                return;

        if (r == 1)
                i = (i%6)+1;

        //gi.dprintf ("Ricochet - r = %i, i = %i\n", r, i);
        
        VectorCopy (point, level.falsetemp->s.origin);
        gi.positioned_sound (point, level.falsetemp, CHAN_AUTO, gi.soundindex(va("world/rico%i.wav", i)), 1, ATTN_NORM, 0);
}
*/
// Dirty
// Paranoid

/*
=================
fire_lead - the proj bit of lead weapons
=================
*/

// Dirty
void lead_free (edict_t *self)
{
        total_lead--;
        G_FreeEdict(self);
}
// Dirty

void lead_touch (edict_t *lead, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        float   r, u, n, kck, dmg;
        int     i, distance;
        vec3_t  impactdir;

        if (surf && (surf->flags & SURF_SKY))
        {
                G_FreeEdict (lead);
                return;
	}

        //VectorMA (lead->s.origin, 2, lead->velocity, impactdir);
        if (gi.pointcontents (lead->s.origin) & MASK_WATER)
        {
                // Dirty
                //G_FreeEdict (lead);
                lead_free (lead);
                // Dirty
                return;
        }

        if (lead->request & TOD_EXPLOSIVE)
        {
                LeadExplode(lead, other, lead->velocity, lead->s.origin, plane->normal, lead->dmg, lead->modtype, lead->request);
                return;
        }

	if (other->takedamage)
	{
                VectorSubtract (lead->s.origin, lead->corpse_pos, impactdir);
                distance = VectorLength (impactdir);

                n = distance;
                u = lead->points;
                r = (n / u);
                n = (1 - r);

                if (n < 0.5)
                        n = 0.5;

                dmg = (lead->dmg * n);
                kck = (lead->wait * n);

                //gi.dprintf("lead_touch : dmg = %f, n = %f\n", dmg, n);

                VectorScale (lead->velocity, 0.2, impactdir);
                T_Damage (other, lead, lead->owner, impactdir, lead->s.origin, plane->normal, dmg, kck, 0, lead->request, lead->modtype);
        }
        else
	{
                if (lead->count > 1)
                        i = rand()%lead->count;
                else
                        i = 1;

                if (i == 1)
                        TakingFireCheck (lead->owner, lead->s.origin);

		gi.WriteByte (svc_temp_entity);
                // Dirty - Less is more
                if (lead->request & TOD_PELLET)
                        gi.WriteByte (TE_SHOTGUN);
                else
                        gi.WriteByte (TE_BULLET_SPARKS);
                //      gi.WriteByte (TE_GUNSHOT);
                // Dirty
                gi.WritePosition (lead->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
                gi.multicast (lead->s.origin, MULTICAST_PVS);

                // Paranoid
                // Dirty Ricochet (lead->s.origin, lead->count);
                // Paranoid

                //VectorMA (lead->s.origin, -1, lead->velocity, impactdir);
                //TakeFromBHoleQue (lead, "models/lag/bholes/bhole1.md2", impactdir, plane->normal);
	}
        // Dirty
        //G_FreeEdict (lead);
        lead_free (lead);
        // Dirty
}

static void fire_lead (edict_t *ent, vec3_t start, vec3_t dir, int inst_range, int damage, int kick, int mod, int type, int count)
{
        edict_t *lead;
        int     range;

        // Dirty - Enforce limit
        if (instant_lead >= INSTANT_LEAD_LIMIT)
                return;

        if (total_lead >= LEAD_LIMIT)
                return;

        instant_lead++;
        total_lead++;
        // Dirty - Enforce limit

        range = 8192 - inst_range;
        if (range < 1024)
                return;

        damage *= 0.5;
        kick *= 0.5;

        lead = G_Spawn();

        lead->classname = "lead";
        lead->owner = ent;

        VectorCopy (start, lead->s.origin);
        VectorCopy (start, lead->corpse_pos);
        VectorScale (dir, 2000, lead->velocity);
        lead->velocity[2] -= 1;

        lead->movetype = MOVETYPE_FLYMISSILE;
        //lead->clipmask = (MASK_SHOT | MASK_WATER);
        lead->clipmask = MASK_SHOT;
        lead->solid = SOLID_BBOX;
        lead->touch = lead_touch;
        lead->svflags |= (SVF_DEADMONSTER | SVF_NOCLIENT);

        lead->nextthink = level.time + 1.5;
        // Dirty
        //lead->think = G_FreeEdict;
        lead->think = lead_free;
        // Dirty

        lead->points = range;
        lead->request = type;
        lead->dmg = damage;
        lead->wait = kick;
        lead->modtype = mod;
        lead->count = count;

        gi.linkentity (lead);
}


/*
================
fire_hitscan - set ignore to thing you want it do go thru.
                NULL mean no penetration

        pref_range = pre-effective range.
                Damage increases as it aproaches this,
                then decreases after it.

        n_range = nominal range.
                added to simulate a curve

        inst_range = distance where there would be no noticable delay
                between pulling the trigger and seeing the impact.
                If it goes beyond this range, it's sent to the tracker,
                who will mimic a projectile by firing another traceline
                in the next frame.
================
*/
static void fire_hitscan (edict_t *ent, int count, vec3_t start, vec3_t end, vec3_t aimdir,
 float pref_range, float n_range, float inst_range, int type, int damage, int kick, int mod)
{
        edict_t         *ignore = NULL;
        vec3_t          true_end, from, dist, water_start;
        qboolean        water = false;
        float           thk, thickness, distance;
        float           dmg, kck, n, r;
        int             max_thru, m, ht, i;
        trace_t         tr;
        int             content_mask = MASK_SHOT | MASK_WATER;
        int             cs = 0;  // Crash Stopper. Cheap huh?  8)
                                // Well, a MAX limit thing to prevent
                                // infinite loops.. just in case

        if (type & TOD_PELLET) // shotgun - count = no# of pellets
                max_thru = 0;
        else // not shotgun - count = max no# of things we can go through
        {
                max_thru = count;
                count = 1;
        }        

        VectorCopy (start, from);
        VectorCopy (end, true_end);
        
        thickness = pref_range / 16;
        if (thickness > 96)
                thickness = 96;
        else if (thickness < 16)
                thickness = 16;

        ignore = ent; // start with us

        m = max_thru;
        while ((m > -1) && (cs < 24))
        {
                cs++;
                if (gi.pointcontents (from) & MASK_WATER)
                {
                        if (!water)
                        {
                                pref_range *= 0.35;
                                n_range *= 0.35;
                                inst_range *= 0.35;
                                VectorCopy (from, water_start);
                                VectorMA (from, inst_range, aimdir, true_end);
                                content_mask &= ~MASK_WATER;
                        }
                        water = true;
                }

                tr = gi.trace (from, NULL, NULL, true_end, ignore, content_mask);

                if (tr.fraction >= 1.0)
                {
                        m = -1;
                        break;
                }

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

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

                                if (!water)
                                {
                                        water = true;
                                        pref_range *= 0.1;
                                        n_range *= 0.1;
                                        inst_range *= 0.1;
                                        damage *= 0.7;
                                        VectorCopy (from, water_start);
                                        VectorMA (from, inst_range, aimdir, true_end);
                                        content_mask &= ~MASK_WATER;
                                }
			}

			// re-trace ignoring water this time
                        tr = gi.trace (water_start, NULL, NULL, true_end, ignore, MASK_SHOT);
		}

                if (tr.surface && (tr.surface->flags & SURF_SKY))
                        break;

                VectorSubtract (tr.endpos, start, dist);
                distance = VectorLength (dist);
                if (distance > inst_range)
                {
                        VectorMA (start, inst_range, aimdir, true_end);
                        fire_lead (ent, true_end, aimdir, inst_range, damage, kick, mod, type, count);
                        m = -1;
                        break;
                }

                if (tr.ent->takedamage)
                {       // Damage based on distance
                        // DTEMP - move after TAKEDAMAGE after finished testing
                        if (distance < pref_range)
                        {
                                if (type & TOD_PELLET) // Shotgun = nasty at close range
                                {
                                        type |= TOD_HV;
                                        dmg = damage;
                                        kck = kick;
                                        r = 1;
                                }
                                else
                                {
                                        r = (distance / pref_range);
                                        n = ((float)damage / 2);
                                        dmg = n + (n * r);
                                        n = ((float)kick / 2);
                                        kck = n + (n * r);
                                }
                        }
                        else if (distance > n_range)
                        {
                                r = (1 - ((distance - n_range) / inst_range));
                                n = ((float)damage / 2);
                                dmg = n + (n * r);
                                n = ((float)kick / 2);
                                kck = n + (n * r);
                                m = -1; // greater then this distance = no going through things
                        }
                        else
                        {
                                m = -1; // greater then this distance = no going through things
                                r = 1;
                                dmg = damage;
                                kck = kick;
                        }

                        //gi.dprintf ("hitscan - dmg = %i, dist = %i\n", (int)dmg, (int)distance);
                        // DTEMP
                        
                        T_Damage (tr.ent, ent, ent, aimdir, tr.endpos, tr.plane.normal, dmg, kck, 0, type, mod);

                        if (type & TOD_EXPLOSIVE)
                        {
                                LeadExplode(ent, tr.ent, aimdir, tr.endpos, tr.plane.normal, damage, mod, type);
                                break;
                        }

                        if ((m < 0) || (HitLocation & HIT_SOAKED))
                                break;

                        if (type & TOD_PELLET) // not for shotguns
                                break;

                        if (tr.ent->client || (tr.ent->svflags & SVF_DEADMONSTER) || (tr.ent->svflags & SVF_MONSTER))
                        {
                                if (count > 1)
                                        i = rand()%count;
                                else
                                        i = 1;
                        }
                        else
                                break;

                        // No blood for shotguns (would be too much of it then)
                        if (i == 1)
                        {
                                //fire_blood (tr.ent, tr.endpos, aimdir, 2000);

                                // Chance of going through someone based on distance n' gun
                                if (r > 1)
                                        r -= 1;

                                // Chance altered by hit location...
                                if (HitLocation & (HIT_LEFT_ARM | HIT_RIGHT_ARM))
                                        r += 0.3;
                                else if (HitLocation & (HIT_LEFT_LEG | HIT_RIGHT_LEG))
                                        r += 0.2;
                                else if (HitLocation & HIT_HEAD)
                                        r -= 0.3;
                                else if (HitLocation & HIT_MID)
                                        r += 0.2;
                                else if (HitLocation & HIT_CHEST)
                                        r -= 0.1;

                                if (HitLocation & HIT_SLOWED)
                                        r -= 0.3;

                                if (type & TOD_PELLET)
                                        r -= 0.2;
                                else if (distance < pref_range)
                                        r += 0.4;

                                if ((r > 0) && (random() < r))
                                {
                                        //gi.dprintf ("hitscan - thru takedamage\n");
                                        pref_range *= 0.9;
                                        n_range *= 0.9;
                                        inst_range *= 0.9;
                                        ignore = tr.ent;
                                        VectorCopy (tr.endpos, from);
                                        m--;
                                }
                                else
                                        break;
                        }
                }
                else
                {
                        if (count > 1)
                                i = rand()%count;
                        else
                                i = 1;
                        
                        if (i == 1)
                                TakingFireCheck (ent, tr.endpos);

                        if (type & TOD_EXPLOSIVE)
                        {
                                LeadExplode(ent, tr.ent, aimdir, tr.endpos, tr.plane.normal, damage, mod, type);
                                break;
                        }

                        // Impact puff
                        gi.WriteByte (svc_temp_entity);
                        // Dirty - different
                        //gi.WriteByte (TE_GUNSHOT);
                        if (type & TOD_PELLET)
                                gi.WriteByte (TE_SHOTGUN);
                        else
                                gi.WriteByte (TE_BULLET_SPARKS);
                        // Dirty
                        gi.WritePosition (tr.endpos);
                        gi.WriteDir (tr.plane.normal);
                        gi.multicast (tr.endpos, MULTICAST_PVS);

                        if ((type & TOD_PELLET) || water)
                                break;

                        // If too far, no penetration
                        if (type & TOD_HV)
                        {
                                if (distance > inst_range)
                                        break;
                        }
                        else if (distance > pref_range)
                                break;

                        if (m < 1)
                                thickness = 4;

                        thk = thickness;

                        if (thk < 2)
                                thickness = 2;

                        VectorCopy (tr.endpos, from);

                        ht = 0;
                        while (thk > 0)
                        {
                                thk -= 2;
                                ht += 2;

                                if (ht > 96)
                                        thk = 0; // just in case

                                VectorMA (from, 2, aimdir, from);
                                if (!(gi.pointcontents(from) & MASK_SOLID))
                                        thk = -1;
                        }

                        if (thk > -1)
                        {
                                m = -1;
                                break; // thats it, we're done
                        }

                        if (ht > 4) // else, its too thin to worry about
                        {           // (we're talking THIN here)
                                damage *= 0.8;
                                kick *= 0.8;
                                m--;
                                //gi.dprintf ("hitscan - thru wall\n");
                        }
                        //else
                                //gi.dprintf ("hitscan - thru thin arse wall\n");
                }
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

                VectorSubtract (tr.endpos, water_start, dist);
                VectorNormalize (dist);
                VectorMA (tr.endpos, -2, dist, pos);
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
fire_spread - Calculates spread, start postition, end position, new direction
              (forward) and finally calls the hitscan function...
=================
*/
static void fire_spread (edict_t *ent, vec3_t start, vec3_t aimdir, float pref_range, float n_range, float inst_range, int hspread, int vspread, int type, int damage, int kick, int mod, int count)
{
        vec3_t  forward, right, up, dir, end;
        float   r, u;

        vectoangles (aimdir, dir);
        AngleVectors (dir, NULL, right, up);

        // Set up point for normal traceline
        VectorMA (start, 8192, aimdir, end);

        r = crandom() * hspread;
        VectorMA (end, r, right, end);

        u = crandom() * vspread;
        VectorMA (end, u, up, end);

        // Set up point for launching projectile.
        VectorSubtract (end, start, dir);
        vectoangles (dir, dir);
        AngleVectors (dir, forward, NULL, NULL);

        fire_hitscan (ent, count, start, end, forward, pref_range, n_range, inst_range, type, damage, kick, mod);
}

/*
=================
fire_baselead - used for any lead based weapons.
        for shotguns, count > 1
        for all others, count = 1;
=================
*/

void fire_baselead (edict_t *ent, vec3_t start, vec3_t aimdir, float pref_range, float n_range, float inst_range, int hspread, int vspread, int type, int damage, int kick, int mod, int count)
{
        int effect, i;

        if (count < 1)
                count = 1;

        if (ent->client && (!(type & TOD_PELLET)))
        {
                // Firing several times in quick succesion reduces accuracy.
                effect = ContEffect (ent);
                if (effect > 1)
                {
                        hspread *= effect;                                
                        vspread *= effect;                                
                }

                if ((xyspeed > 100) || (ent->client->weapon_side_set && (ent->client->weapon_side_set >= xyspeed))
                 || (ent->client->weapon_forward_set && (ent->client->weapon_forward_set >= xyspeed)))
                {
                        hspread *= 2;
                        vspread *= 1.5;                                
                }
                else if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                {
                        hspread *= 0.5;
                        vspread *= 0.5;
                }

                // Dirty
                if ((ent->client->pers.specials & SI_LASER_SIGHT_HELP) && (effect < 3))
                {
                        hspread *= 0.7;
                        vspread *= 0.65;
                }

                if (ent->client->held_shots > 1)
                {
                        hspread = (hspread / ent->client->held_shots);
                        vspread = (vspread / ent->client->held_shots);
                        ent->client->held_shots = 0; // Can only be ONE held shot
                }

                if (!ent->groundentity) // Good idea?
                {
                        hspread *= 1.2;
                        vspread *= 1.2;
                }

                // Two seperate weapon = less accuracy for both...
                if (ent->client->pers.weapon_handling)
                {
                        hspread *= 1.2;
                        vspread *= 1.1;
                }

                // Spread capper... bullets hitting your feet is over the top
                // FIX ME - remove?
                if (vspread > 768)
                        vspread = 768;
                if (hspread > 768)
                        hspread = 768;
                // Dirty

                damage += (crandom()*4) + 1;
        }

        if (ent->client && (ent->client->quad_framenum > level.framenum))
        {
                damage *= 4;
                kick *= 4;
        }

        // Dirty - FIX ME! Make me nicer
        if (ent->client && ent->client->pers.weapon_handling && (ent->client->pers.weapon == ent->client->pers.weapon2))
                mod++;

        if (type & TOD_PELLET)
        {
                for (i = 0; i < count; i++)
                        fire_spread (ent, start, aimdir, pref_range, n_range, inst_range, hspread, vspread, type, damage, kick, mod, count);
        }
        else
                fire_spread (ent, start, aimdir, pref_range, n_range, inst_range, hspread, vspread, type, damage, kick, mod, count);
        // Dirty
}

/*
=================
GRIM END OF LEAD CODE
=================
*/


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
                /* GRIM
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
                */
                        mod = MOD_BLASTER;
                // GRIM
                        
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, TOD_NO_WOUND, mod);
	}
	else
	{
                TakingFireCheck (self->owner, self->s.origin); // GRIM - 15/01/99
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
MULTIPLE GRENADES
=================
*/
// GRIM
/*
=================
Calc_Arc - This is a support routine for keeping an object velocity-aligned
           I.E for arrows/darts
=================
*/

void Calc_Arc (edict_t *ent)
{
        vec3_t move;

        vectoangles(ent->velocity, move);
        VectorSubtract(move, ent->s.angles, move);

        move[0] = fmod((move[0] + 180), 360) - 180;
        move[1] = fmod((move[1] + 180), 360) - 180;
        move[2] = fmod((move[2] + 180), 360) - 180;
        VectorScale(move, 1/FRAMETIME, ent->avelocity);
}

int GrenMod (edict_t *grenade, qboolean splash)
{
        if (splash)
        {
                if (grenade->count & GREN_HELD)
                        return MOD_HELD_GRENADE;
                else if (grenade->count & GREN_HELD)
                        return MOD_HG_SPLASH;
                else
                        return MOD_G_SPLASH;
        }
        else if (grenade->count & GREN_HAND)
                return MOD_HANDGRENADE;
        else
                return MOD_GRENADE;
}

/*
=================
Cluster_Explode - Fires off four more grenades...
=================
*/
void Cluster_Explode (edict_t *ent)
{
        vec3_t  grenade1, grenade2, grenade3, grenade4;

	// give grenades up/outwards velocities
        VectorSet (grenade1, 20, 20, 40);
        VectorSet (grenade2, 20, -20, 40);
        VectorSet (grenade3, -20, 20, 40);
        VectorSet (grenade4, -20, -20, 40);

	// explode the four grenades outwards
        fire_grenade2 (ent->master, ent->s.origin, grenade1, CLUSTER_DAMAGE, 10, CLUSTER_TIME, CLUSTER_DAMAGE, 0);
        fire_grenade2 (ent->master, ent->s.origin, grenade2, CLUSTER_DAMAGE, 10, CLUSTER_TIME, CLUSTER_DAMAGE, 0);
        fire_grenade2 (ent->master, ent->s.origin, grenade3, CLUSTER_DAMAGE, 10, CLUSTER_TIME, CLUSTER_DAMAGE, 0);
        fire_grenade2 (ent->master, ent->s.origin, grenade4, CLUSTER_DAMAGE, 10, CLUSTER_TIME, CLUSTER_DAMAGE, 0);
}

/*
==================
Flash Grenade - Blinds all those within radius....
==================
*/
void SpawnWeld (vec3_t origin, vec3_t normal);
void flare_think (edict_t *ent)
{
        ent->nextthink = level.time + 0.1;

        if (ent->attack_time < level.time)
                ent->think = G_FreeEdict;
        else
        {
                if (ent->s.effects & EF_HYPERBLASTER)
                {
                        ent->s.effects &= ~EF_HYPERBLASTER;
                        ent->s.effects |= EF_BLASTER;
                }
                else
                {
                        ent->s.effects &= ~EF_BLASTER;
                        ent->s.effects |= EF_HYPERBLASTER;
                        //SpawnWeld (ent->s.origin, ent->s.angles);
                }
        }
}

void Flash_Explode (edict_t *ent)
{
	edict_t *target;
        vec3_t  dir;
        float   effect;
        float   calc, dist;

        target = NULL;

        while ((target = findradius(target, ent->s.origin, ent->dmg_radius)) != NULL)
        {
                // You know when to close your eyes, don't you?
                // GRIM = SFW? Just half for owner...
                //if (target == ent->owner)
                //        continue;
                // GRIM 

                // It's not a player
                if (!target->client)
                        continue;

                // The grenade can't see it
                if (!visible(ent, target))
                        continue;

                // It's not facing it
                // GRIM - 2/3 effect..
                if (!infront(target, ent))
                        effect = 10;
                else
                        effect = 15;

                VectorSubtract (target->s.origin, ent->s.origin, dir);
                dist = VectorLength (dir);
                calc = 1 - (dist / (float)ent->dmg_radius);
                if (dist < 32)
                        calc = 1;
                effect *= calc;

                if (effect < 3)
                        effect = 3;

                // Increment the blindness counter
                if (target->blindTime > level.time)
                        target->blindTime += effect;
                else
                        target->blindTime = level.time + effect;

                // GRIM - Fuck telling the ppl..let em' work it out.
                /*
                // Let the player know what just happened
                // (It's just as well, he won't see the message immediately!)
                gi.cprintf(target, PRINT_HIGH, "You are blinded by a flash grenade!\n");

                // Let the owner of the grenade know it worked
                gi.cprintf(ent->owner, PRINT_HIGH, "%s is blinded by your flash grenade!\n", target->client->pers.netname);
                */
                // GRIM - Fuck telling the ppl..let em' work it out.
        }


        ent->s.effects |= EF_COLOR_SHELL;
        ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
        ent->think = flare_think;
        ent->nextthink = level.time + 0.1;
        ent->s.sound |= EF_HYPERBLASTER;
        ent->s.sound = gi.soundindex ("misc/lasfly.wav");
        ent->s.effects |= EF_HYPERBLASTER;
        ent->attack_time = level.time + 60;
        gi.linkentity (ent);
}

/*
=================
fire_grenade
=================
*/
void Grenade_Explode (edict_t *ent)
{
        vec3_t  origin;
        int     mod, radius;

        if (ent->master && (ent->master->client))
                PlayerNoise(ent->master, ent->s.origin, PNOISE_IMPACT);

        mod = GrenMod(ent, false);

        if (ent->dmg)
        {
                // Instead of enemy coz otherwise it damage the person who blew it up!
                if (ent->activator)
                        T_Damage (ent->activator, ent, ent->master, ent->velocity, ent->s.origin, vec3_origin, ent->dmg, ent->dmg, 0, (TOD_RADIUS | TOD_EXPLOSIVE), mod);

                mod = GrenMod(ent, true);

                radius = ent->dmg_radius;

                if (radius)
                        T_RadiusDamage(ent, ent->master, ent->dmg, ent->activator, radius, mod);
        }

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

        if (ent->count & GREN_CLUSTER)
                Cluster_Explode(ent);
        else if (ent->count & GREN_FLASH)
                Flash_Explode (ent);
        else
                G_FreeEdict (ent);
}

void Grenade_Die (edict_t *ent, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
       ent->takedamage = DAMAGE_NO;
       ent->nextthink = level.time + 0.1;
       ent->think = Grenade_Explode;
       gi.linkentity (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

        if (ent->watertype & CONTENTS_LAVA)
        {
                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/grenlx1a.wav"), 1, ATTN_NORM, 0);
                ent->think(ent);
                return;
        }

        VectorSet (ent->avelocity, 300, 300, 300);
        if (ent->prethink)
                ent->prethink = NULL;

        // Hit a wall...
	if (!other->takedamage)
	{
                TakingFireCheck (ent->master, ent->s.origin); // GRIM - 15/01/99
                if (ent->count & GREN_CONTACT)
                        Grenade_Explode (ent);
                else if (ent->count & GREN_HAND)
                {
                        if (random() > 0.5)
                                gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
                        else
                                gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
                }
                else
                {       // Launched grenades are movetype flymissile...
                        ent->movetype = MOVETYPE_BOUNCE;
                        gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
                }
                return;
	}

        // Grenades set to contact only explode if armed.
        if ((ent->spawnflags & 4) && (ent->count & GREN_CONTACT))
	{
		ent->activator = other;
		Grenade_Explode (ent);
	}

        ent->movetype = MOVETYPE_TOSS;

        gi.linkentity (ent);
}

/*
void Grenade_Arm (edict_t *ent)
{
        ent->nextthink = level.time + ent->weight;

        ent->think = Grenade_Explode;
        ent->owner = NULL;

        if (ent->count & GREN_CONTACT)
                ent->spawnflags = 4;

        gi.linkentity (ent);
}
*/

/*
=================
fire_grenade
=================
*/
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int type)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
        VectorScale (aimdir, speed, grenade->velocity);
        vectoangles (aimdir, grenade->s.angles);

        VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        //VectorMA (grenade->velocity, 50 + crandom() * 10.0, up, grenade->velocity);

        //grenade->movetype = MOVETYPE_FLYMISSILE;
        grenade->movetype = MOVETYPE_BOUNCE;
        grenade->prethink = Calc_Arc;

        VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
        grenade->prethink = Calc_Arc;

        grenade->clipmask = MASK_SHOT;
        grenade->solid = SOLID_BBOX;

        // Even though it noclips bboxs, it still can be shot!
        grenade->svflags |= SVF_DEADMONSTER;

        grenade->s.effects |= EF_GRENADE;
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");

        grenade->master = self;
        //grenade->owner = self;

	grenade->touch = Grenade_Touch;

        // Count doubles as type
        grenade->count = type;

        //grenade->think = Grenade_Arm;
        //grenade->nextthink = level.time + 0.2;
        //grenade->weight = timer - 0.1;
        grenade->think = Grenade_Explode;
        grenade->nextthink = level.time + timer;

	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

        VectorSet(grenade->mins, -6, -6, -6);
        VectorSet(grenade->maxs, 6, 6, 6);

        grenade->mass = 2;
        grenade->health = 1;
        grenade->die = Grenade_Die;
        grenade->takedamage = DAMAGE_YES;
        grenade->monsterinfo.aiflags = AI_NOSTEP;
        gi.linkentity (grenade);

	if (self->client)
                check_dodge (self, grenade->s.origin, aimdir, speed);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int type)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);

        // Dirty
        if (self->health < 1)
                grenade->s.origin[2] += 8;
                
        if (speed >= 100)
        {
                VectorScale (aimdir, speed, grenade->velocity);
                VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
        }
        else
        {
                VectorScale (aimdir, 48, grenade->velocity);
                VectorMA (grenade->velocity, 20, up, grenade->velocity);
        }
        // Dirty

	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
        grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;

        // Even though it noclips bboxs, it still can be shot!
        grenade->svflags |= SVF_DEADMONSTER;

	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);

        grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");

        grenade->master = self;
        //grenade->owner = self;

	grenade->touch = Grenade_Touch;

        //grenade->think = Grenade_Arm;
        //grenade->nextthink = level.time + 0.2;
        //grenade->weight = timer - 0.1;
        grenade->think = Grenade_Explode;
        grenade->nextthink = level.time + timer;

	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";

        // Count doubles as type
        grenade->count = type;

        // GRIM - Fuck off with the sound
        //grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

        VectorSet(grenade->mins, -4, -4, -4);
        VectorSet(grenade->maxs, 4, 4, 4);

        grenade->mass = 2;
        grenade->health = 1;
        grenade->die = Grenade_Die;
        grenade->takedamage = DAMAGE_YES;
        grenade->monsterinfo.aiflags = AI_NOSTEP;

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
                // Dirty
                if (speed >= 100)
                        gi.sound (self, CHAN_GOODHAND, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
                // Dirty
		gi.linkentity (grenade);
	}

        //if (self->client)
        //        check_dodge (self, grenade->s.origin, aimdir, speed);
}
// GRIM - MULTIPLE GRENADES


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
                T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, ent->dmg, 0, (TOD_RADIUS | TOD_EXPLOSIVE), MOD_ROCKET);
	else
	{
                TakingFireCheck (ent->owner, ent->s.origin); // GRIM - 15/01/99
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

// GRIM
        if (ent->watertype & CONTENTS_LAVA)
                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/grenlx1a.wav"), 1, ATTN_NORM, 0);
// GRIM

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
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
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
// GRIM
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
        int                     n = 0;
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
                        if (tr.ent->takedamage)
                        {
                                T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, TOD_AP, MOD_RAILGUN);
                                // If they soak the damage (Not likely), stop
                                if (HitLocation & HIT_SOAKED)
                                        ignore = NULL;
                                else
                                        ignore = tr.ent;

                                if ((ignore->svflags & SVF_MONSTER) || (ignore->client))
                                        fire_blood (ignore, tr.endpos, aimdir, 2000);
                        }
                        else
                                ignore = NULL;
		}

		VectorCopy (tr.endpos, from);
	}

        TakingFireCheck (self, tr.endpos); // GRIM - 15/01/99

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
        /*
        VectorMA (tr.endpos, -1, aimdir, from);
        TakeFromBHoleQue (self, "models/lag/bholes/railhole.md2", from, tr.plane.normal);
        */
        if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

        VectorMA (tr.endpos, 1, aimdir, from);
        while (n < 128)
        {
                if ((!(gi.pointcontents(from) & CONTENTS_SOLID)) )
                {
                        fire_rail (self, from, aimdir, damage, kick);
                        
                        gi.WriteByte (svc_temp_entity);
                        gi.WriteByte (TE_GUNSHOT);
                        gi.WritePosition (from);
                        gi.WriteDir (tr.plane.normal);
                        gi.multicast (tr.endpos, MULTICAST_PVS);
                        
                        /*
                        tr = gi.trace (from, NULL, NULL, tr.endpos, ignore, mask);
                        VectorMA (tr.endpos, 1, aimdir, from);
                        TakeFromBHoleQue (self, "models/lag/bholes/railhole.md2", from, tr.plane.normal);
                        */
                        if (!deathmatch->value && !coop->value)
                        {
                                n = rand() % 3;
                                while(n--)
                                        ThrowDebris (self, "models/objects/debris2/tris.md2", 30, tr.endpos);
                        }
                        n = 128;
                }
                else
                        n++;
                VectorMA (from, 1, aimdir, from);
        }
}
// GRIM


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
                        T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, 0, 0, MOD_BFG_EFFECT);
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
                T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, 0, MOD_BFG_BLAST);
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
        TakingFireCheck (self->owner, self->s.origin); // GRIM - 15/01/99
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
                                T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, 0, 0, MOD_BFG_LASER);

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

        TakingFireCheck (self, start); // GRIM - 15/01/99

	gi.linkentity (bfg);
}
