#include "g_local.h"

/*
===========================
Gas Grenades
===========================
*/

void Gas_Explode (edict_t *ent);

void Gas_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Gas_Explode;
}

void Gas_Explode (edict_t *ent)
{
	vec3_t		origin,v;
	edict_t *target;
	float Distance, DrunkTimeAdd;	

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

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
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
    target = NULL;
    while ((target = findradius(target, ent->s.origin, 80)) != NULL)
     {
        if (!target->client)
            continue;       // It's not a player

        if (target->wf_team == ent->wf_team)    //don't attack if on same team
            continue;

        if (!visible(ent, target))
            continue;       // The grenade can't see it

		// Find distance
		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);
		// Calculate drunk factor
		if(Distance < 80/10)
			DrunkTimeAdd = 10; //completely drunk
        else
            DrunkTimeAdd = 1.5 * 10 * ( 1 / (( Distance - 80 ) / (80*2) - 2 ) + 2 ); //partially drunk
        if ( DrunkTimeAdd < 0 )
            DrunkTimeAdd = 0; // Do not make drunk at all.

              
        // Increment the drunk time
        if(target->client->Gas_Time < level.time)
			target->client->Gas_Time = DrunkTimeAdd+level.time;
		else
			target->client->Gas_Time += DrunkTimeAdd/3;               
	}

	// shake view
	T_ShockWave(ent, 255, 1024);
	// explode and destroy grenade
	BecomeNewExplosion (ent);	
}

void Gas_think(edict_t *ent)
{
	vec3_t	v;
	edict_t *target;
	float Distance,DrunkTimeAdd;
	vec3_t 	forward, right, up;
	ent->s.angles[0]+=10*random()-5;
	ent->s.angles[1]+=10*random()-5;
	ent->s.angles[2]+=10*random()-5;
	AngleVectors (ent->s.angles, forward, right, up);
	ent->movetype = MOVETYPE_FLYRICOCHET;
	VectorSet (ent->avelocity, 150, 150, 150);
	//VectorMA (ent->velocity, 200 + crandom() * 10.0, up, ent->velocity);
	//VectorMA (ent->velocity, crandom() * 10.0, right, ent->velocity);
	
    target = NULL;
    while ((target = findradius(target, ent->s.origin, 220)) != NULL)
    {
        if (!target->client)
            continue;       // It's not a player

        if (target->wf_team == ent->wf_team)    //don't attack if on same team
            continue;

        if (!visible(ent, target))
            continue;       // The grenade can't see it


		// Find distance
		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);
		// Calculate drunk factor
		if(Distance < 220/10)
			DrunkTimeAdd = 10; //completely drunk
        else
            DrunkTimeAdd = 1.5 * 10 * ( 1 / (( Distance - 220 ) / (220*2) - 2 ) + 2 ); //partially drunk
        if ( DrunkTimeAdd < 0 )
            DrunkTimeAdd = 0; // Do not make drunk at all.
        if ( DrunkTimeAdd > 10 )
            DrunkTimeAdd = 10; // Do not make drunk at all.

              
        // Increment the drunk time
        if(target->client->Gas_Time < level.time)
			target->client->Gas_Time = DrunkTimeAdd+level.time;
		else
			target->client->Gas_Time += DrunkTimeAdd/3;               
	}
	//Spray out some sparks at random
	if(random()<0.30)
	{
		//Gregg add in some type of air sound to be played
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHIELD_SPARKS);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir (ent->s.angles);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		//The gas coming out pushes the grenade around
		VectorScale (forward, 50+((int)(random()*1000)%150), ent->velocity);
	}
	//Some more sparks
	if(random()>0.25)
	{
		//Gregg add in some type of air sound to be played
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SCREEN_SPARKS);
		gi.WritePosition (ent->s.origin);
		gi.WriteDir (ent->s.angles);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		//The gas coming out pushes the grenade around
		VectorScale (forward, 40+((int)(random()*1000)%160), ent->velocity);
	}
	//Some really wierd sparks but sprays out a lot more of the plague
	if(random()<0.15)
	{
		//Gregg add in some type of air sound to be played
		/*gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BOSSTPORT);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		*/
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_WIDOWBEAMOUT);
		gi.WriteShort (2000);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PHS);
		target = NULL;
		while ((target = findradius(target, ent->s.origin, 80)) != NULL)
		{
			if (!target->client)
				continue;       // It's not a player

                        if (target->wf_team == ent->wf_team)    //don't attack if on same team
                              continue;

			if (!visible(ent, target))
				continue;       // The grenade can't see it


			// Find distance
			VectorSubtract(ent->s.origin, target->s.origin, v);
			Distance = VectorLength(v);
			// Calculate drunk factor
			if(Distance < 80/10)
				DrunkTimeAdd = 10; //completely drunk
			else
				DrunkTimeAdd = 1.5 * 10 * ( 1 / (( Distance - 80 ) / (80*2) - 2 ) + 2 ); //partially drunk
			if ( DrunkTimeAdd < 0 )
				DrunkTimeAdd = 0; // Do not make drunk at all.

              
			// Increment the drunk time
			if(target->client->Gas_Time < level.time)
				target->client->Gas_Time = DrunkTimeAdd+level.time;
			else
				target->client->Gas_Time += DrunkTimeAdd/3;               
		}
	}

	if(ent->delay<level.time)
                ent->think=Gas_Explode;
	ent->nextthink=level.time + 0.5;	
}
void Gas_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	// Dont blow up if on same team
	if (other->wf_team == ent->wf_team)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		//ent->enemy = other;
		Gas_Explode(ent);
	}
}



void fire_gasgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	++self->client->pers.active_grenades[GRENADE_TYPE_GAS];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_GAS;
	grenade->wf_team = self->wf_team;
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_FLYRICOCHET;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Gas_Touch; 
	grenade->nextthink = level.time + 1;
	grenade->delay = level.time + 10;
	grenade->think = Gas_think; 
	grenade->dmg = (damage/3);
	grenade->dmg_radius = damage_radius;
	grenade->classname = "gas";
	// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = Gas_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}
