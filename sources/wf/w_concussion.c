#include "g_local.h"

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
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

    target = NULL;
    while ((target = findradius(target, ent->s.origin, 520)) != NULL)
    {
        if (!target->client)
            continue;       // It's not a player

        if (target->wf_team == ent->wf_team)    //don't attack if on same team
            continue;

        if (!visible(ent, target))
            continue;       // The grenade can't see it

		if (target->DrunkTime > level.time)
			continue;		//don't make them more drunk (Gregg)

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

		//GREGG - decrease drunk time
		DrunkTimeAdd *= .3;
              
        // Increment the drunk time
        if(target->DrunkTime < level.time)
			target->DrunkTime = DrunkTimeAdd+level.time;
		else
			target->DrunkTime += DrunkTimeAdd;               
	}

   // Blow up the grenade
   BecomeExplosion1(ent);
}

void Concussion_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Concussion_Explode;
}


void Concussion_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	//ent->enemy = other;  was causing damage to thrower?
	Concussion_Explode (ent);
}


void fire_concussiongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	++self->client->pers.active_grenades[GRENADE_TYPE_CONCUSSION];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_CONCUSSION;
	grenade->wf_team = self->wf_team;

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
	grenade->s.modelindex = gi.modelindex (GRCONCUSSION_MODEL);
	grenade->s.skinnum = GRCONCUSSION_SKIN;
	grenade->owner = self;
	grenade->touch = Concussion_Touch; 
	grenade->nextthink = level.time + timer;
	grenade->think = Concussion_Explode; 
	grenade->dmg = wf_game.grenade_damage[GRENADE_TYPE_CONCUSSION];
	grenade->dmg_radius = damage_radius;
	grenade->classname = "concussion";
			// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = Concussion_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}
