/*==============================================================================
The Weapons Factory - 
Napalm Grenade Functions
Original code by ??
Modified by Gregg Reno
==============================================================================*/
#include "g_local.h"

static void Napalm_Explode (edict_t *ent)

{
	vec3_t      origin;
	edict_t		*blip;

	//Sean added these 4 vectors

	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;
	vec3_t   grenade5;

	if (ent->owner->client)
	{
//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("jumppack.wav"), 1, ATTN_NORM, 0);
	}

	//No damage for now
	//T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_WF_CLUSTER);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	// SumFuka did this bit : give grenades up/outwards velocities
	VectorSet(grenade1,20,20,40);
	VectorSet(grenade2,20,-20,40);
	VectorSet(grenade3,-20,20,40);
	VectorSet(grenade4,-20,-20,40);

	VectorSet(grenade5,0,0,40);

	// Sean : explode the four grenades outwards
	// Arguments: fire_grenade2 (self, start, aimdir, damage, speed, timer, damage_radius, held)
	//fire_flame(ent->owner, origin, grenade1, ent->dmg, 8, 1.0, 120, false);
	//fire_flame(ent->owner, origin, grenade2, ent->dmg, 8, 1.0, 120, false);
	//fire_flame(ent->owner, origin, grenade3, ent->dmg, 8, 1.0, 120, false);
	//fire_flame(ent->owner, origin, grenade4, ent->dmg, 8, 1.0, 120, false);

	//See if there is anything to catch on fire
	blip = NULL;
    while ((blip = findradius(blip, ent->s.origin, 150)) != NULL)
        {
        if (!(blip->svflags & SVF_MONSTER) && !blip->client)
            continue;
        if (blip == ent->owner)
            continue;
 	    //dont attack same team
		if (blip->wf_team == ent->wf_team)
		    continue;
        if (!blip->takedamage)
            continue;
        if (blip->health <= 0)
            continue;
        if (!visible(ent, blip))
            continue;

		burn_person(blip, ent->owner, wf_game.grenade_damage[GRENADE_TYPE_NAPALM], MOD_NAPALMGRENADE);
        break;
        }


	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

void Napalm_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        //gi.dprintf("Napalm: Touch\n");

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

    Napalm_Explode (ent);
}


void fire_napalm (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	int dmg;
	vec3_t	forward, right, up;

	++self->client->pers.active_grenades[GRENADE_TYPE_NAPALM];

	//Reduce damage of Napalm
	dmg = damage / 2;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_NAPALM;
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	//grenade->s.effects |= EF_GRENADE;
	grenade->s.effects |=  EF_BFG | EF_HYPERBLASTER;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex (GRNAPALM_MODEL);
	grenade->s.skinnum = GRNAPALM_SKIN;
	grenade->owner = self;
	grenade->touch = Napalm_Touch;

	grenade->nextthink = level.time + timer;
	grenade->think = Napalm_Explode;
	grenade->dmg = dmg;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "napalm grenade";
	grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	//set the team
	grenade->wf_team = self->client->resp.ctf_team;

	if (timer <= 0.0)
		Napalm_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

