#include "g_local.h"

void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);

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
		if (random() < .5)
            G_SplashEntity (TE_LASER_SPARKS, 2400, v, vec3_origin, 0x0000000C, MULTICAST_PVS);
		else
            G_SplashEntity (TE_LASER_SPARKS, 2400, v, vec3_origin, 0x0000000A, MULTICAST_PVS);

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
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_BOSSTPORT);
   gi.WritePosition (origin);
   gi.multicast (ent->s.origin, MULTICAST_PHS);      


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

void fire_smoke_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->think = Smoke_Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
   	grenade->classname = "smoke_grenade";

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
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "pipebomb";
	gi.linkentity (grenade);
}

void Cluster_Explode (edict_t *ent)
{
	vec3_t		origin;

	//Sean added these 4 vectors

	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	// SumFuka did this bit : give grenades up/outwards velocities
	VectorSet(grenade1,20,20,40);
	VectorSet(grenade2,20,-20,40);
	VectorSet(grenade3,-20,20,40);
	VectorSet(grenade4,-20,-20,40);

	// Sean : explode the four grenades outwards
	fire_grenade3(ent, origin, grenade1, 120, 10, 2.0, 120);
	fire_grenade3(ent, origin, grenade2, 120, 10, 2.0, 120);
	fire_grenade3(ent, origin, grenade3, 120, 10, 2.0, 120);
	fire_grenade3(ent, origin, grenade4, 120, 10, 2.0, 120);

    Grenade_Explode (ent);
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
		{
			if (strcmp (target->classname, "sentry"))
			{
				target->enemy = NULL;
				target->goalentity = NULL;
			}
            else
				continue;       // It's not a player
		}

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
    
              
        // Increment the drunk time
        if(target->DrunkTime < level.time)
			target->DrunkTime = DrunkTimeAdd+level.time;
		else
			target->DrunkTime += DrunkTimeAdd;               
	}

   // Blow up the grenade
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_BOSSTPORT);
   gi.WritePosition (ent->s.origin);
   gi.multicast (ent->s.origin, MULTICAST_PHS);
   
   G_FreeEdict (ent);
}

void fire_concussiongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

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
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch; //Stuff for cluster grenades when they explode
	grenade->nextthink = level.time + timer;
	grenade->think = Concussion_Explode; //stuff for cluster grenades exploding
	grenade->dmg = 0;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "concussion";
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->takedamage = DAMAGE_NO;

	gi.linkentity (grenade);
}

void plasma_explode (edict_t *ent)
{
	ent->nextthink = level.time + FRAMETIME;
    ent->s.frame++;
    
    if (ent->s.frame == 5)
		ent->think = G_FreeEdict;
}

void explode_plasma (entity *ent)
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

    gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_BFG_BIGEXPLOSION);
    gi.WritePosition (ent->s.origin);
    gi.multicast (ent->s.origin, MULTICAST_PVS);
	G_FreeEdict (ent);
}

//////////////////////////////////////////
#ifdef LAN
void Zylon_Touch(edict_t *Zylon, edict_t *target, cplane_t *plane, csurface_t *surf)
{ 
	vec3_t dir = {0,0,0}; 

	Zylon->enemy = target; 

	if (target->takedamage) 
		T_Damage(target, Zylon, Zylon->owner, dir, Zylon->s.origin, plane->normal, 20, 0, 0, MOD_ZYLON_GAS );

	G_FreeEdict(Zylon); // free up this cloud upon touch.. 
} 

//====================================================== 
void Generate_Zylon_Gas(edict_t *ent, vec3_t last_angles)
{ 
	edict_t *Zylon; 
	int x,y; 

	Zylon = G_Spawn(); 
    Zylon->classname = "Zylon"; 
	Zylon->owner = ent; // owner is the grenade.. 
	// player is grenade's owner! 

	VectorCopy (ent->s.origin, Zylon->s.origin); 
	x = (random() > 0.5? - 1:1); 
	y = (random() > 0.5? - 1:1); 
	Zylon->s.origin[0] += (random() * 20 + 1) * x; 
	Zylon->s.origin[1] += (random() * 20 + 1) * y; 
	Zylon->s.origin[2] += 8; 
	VectorCopy (ent->s.old_origin, Zylon->s.old_origin); 
	VectorClear(Zylon->s.angles); 
	Zylon->velocity[2] = (random() * 40) + 40; 
	Zylon->velocity[1] = ((int)((random() * 40) + 20 + last_angles[1]) % 60) * y; 
	Zylon->velocity[0] = ((int)((random() * 40) + 20 + last_angles[0]) % 60) * x; 
	VectorCopy(Zylon->velocity, last_angles); 

	Zylon->movetype = MOVETYPE_FLY; // clouds float gently around.. 
	Zylon->solid = SOLID_BBOX; // enable touch detection.. 
    Zylon->s.effects = EF_COLOR_SHELL; // change this to see what you get!! 
	Zylon->s.renderfx = RF_SHELL_GREEN; // gas clouds have green glow.. 

	VectorSet(Zylon->mins, -10, -10, -10); // size of bbox for touch 
	VectorSet(Zylon->maxs, 10, 10, 10); // size of bbox for touch 

	Zylon->s.modelindex = gi.modelindex("sprites/s_explod.sp2"); 
    Zylon->touch = Zylon_Touch; // Touch detection function. 

	Zylon->nextthink = level.time + 10; 
	Zylon->think = G_FreeEdict; // kill gas cloud in 10 secs if not touched.. 
	
	gi.linkentity (Zylon); 
} 

//====================================================== 
void Zylon_Grenade(edict_t *ent)
{ 
	Generate_Zylon_Gas(ent, ent->move_angles); 

	if (ent->Zylon_timer > level.time)
	{ 
		ent->nextthink = level.time + 0.2; 
		ent->think = Zylon_Grenade; 
		return;
	} 

	G_FreeEdict(ent); 
} 
#endif