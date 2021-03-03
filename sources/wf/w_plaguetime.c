#include "g_local.h"
/*
===============
Plague Grenades
===============
*/

void timeDiseaseGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t *target;
//	int rnd;

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
//		--ent->owner->client->pers.active_grenades[GRENADE_TYPE_PLAGUE];

	}
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
//	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,0);

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
	target= NULL;
	
	while ((target = findradius(target, ent->s.origin, 95)) != NULL)
	{
		//Don't go through walls
		if (!visible(ent, target))
             continue;

		//Give them only a 1/3 chance of being infected
		//rnd = rndnum(1,3);
		//if (rnd == 1)
		infect_person(target, ent->owner);	
	}

	//Just infect, don't damage
//	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,0);

// make some debris
//	make_debris (ent);
	// shake view
	T_ShockWave(ent, 255, 1024);
	// let blast move items
//	T_ShockItems(ent);
	// explode and destroy grenade
	BecomeNewExplosion (ent);	
}

void timeDieseaseGrenade_think(edict_t *ent)
{
//	vec3_t	origin;
	edict_t *target;
	int rnd;
	vec3_t 	forward, right, up;
	ent->s.angles[0]+=10*random()-5;
	ent->s.angles[1]+=10*random()-5;
	ent->s.angles[2]+=10*random()-5;
	AngleVectors (ent->s.angles, forward, right, up);
	ent->movetype = MOVETYPE_FLYRICOCHET;
	VectorSet (ent->avelocity, 150, 150, 150);
	//VectorMA (ent->velocity, 200 + crandom() * 10.0, up, ent->velocity);
	//VectorMA (ent->velocity, crandom() * 10.0, right, ent->velocity);
	
	target=NULL;
	while ((target = findradius(target, ent->s.origin, 575)) != NULL)
	{
		//Don't go through walls
		if (!visible(ent, target))
             continue;

		//Give them only a 1/3 chance of being infected
		rnd = ((int)(random()*1000)%3);
		if (!rnd)
		infect_person(target, ent->owner);	
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
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BOSSTPORT);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		while ((target = findradius(target, ent->s.origin, 450)) != NULL)
		{
			//Don't go through walls
			if (!visible(ent, target))
	        	     continue;
			//Give them only a 1/4 chance of being infected
			rnd = (int)(random()*1000)%4;
			if (!rnd)
			infect_person(target, ent->owner);	
		}
	}

	if(ent->delay<level.time)
		ent->think=timeDiseaseGrenade_Explode;
	ent->nextthink=level.time + 0.5;	
}
void timeDiseaseGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
//		if (ent->owner->client)
//			--ent->owner->client->pers.active_grenades[GRENADE_TYPE_PLAGUE];
		G_FreeEdict (ent);
		return;
	}

	if (other->takedamage)
	{
		ent->enemy = other;
		DiseaseGrenade_Explode(ent);
	}
}



void fire_timediseasegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

//	++self->client->pers.active_grenades[GRENADE_TYPE_PLAGUE];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
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
	grenade->s.modelindex = gi.modelindex (GRPLAGUETIME_MODEL);
	grenade->s.skinnum = GRPLAGUETIME_SKIN;
	grenade->owner = self;
	grenade->touch = timeDiseaseGrenade_Touch; 
	grenade->nextthink = level.time + 1;
	grenade->delay = level.time + 10;
	grenade->think = timeDieseaseGrenade_think; 
	grenade->dmg = (damage/3);
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = GenericGrenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;
	grenade->wf_team = self->wf_team;

	gi.linkentity (grenade);
}
