/*==============================================================================
The Weapons Factory -
Proximity Bomb Functions
Original code by (??)
Modified by Gregg Reno
==============================================================================*/
#include "g_local.h"


void Proximity_Explode (edict_t *ent)
{
	vec3_t		origin;

	//gi.dprintf("Proxim: Explode\n");
	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}


	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_WF_PROXIMITY);

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

	G_FreeEdict (ent);
}

// CCH: When a grenade 'dies', it blows up next frame
void Proximity_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//gi.dprintf("Proxim: Die\n");

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Proximity_Explode;
}

void Proximity_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//gi.dprintf("Proxim: Touch\n");

	//if (other == ent->owner)
	//	return;

	// Dont blow up if on same team
	if (other->wf_team == ent->wf_team)
		return;

	if (other->disguised)
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

	//ent->enemy = other;
	Proximity_Explode (ent);
}

// CCH: New think function for proximity grenades
void proxim_think (edict_t *ent)
{
    edict_t *blip = NULL;

    if (level.time > ent->delay)
    {
        Proximity_Explode(ent);
        return;
    }

	//is it armed yet?
    if (level.time < ent->delay2)
	{
	    ent->nextthink = level.time + .1;
		return;
	}

    ent->think = proxim_think;

    while ((blip = findEnemyWithinRadius(ent, blip, ent->s.origin, 100)) != NULL)
    {
        ent->think = Proximity_Explode;
        break;
    }

     ent->nextthink = level.time + .1;
}

void fire_proximity (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int proximity_type)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	int dmg;

	++self->client->pers.active_grenades[GRENADE_TYPE_PROXIMITY];

	//reduce damage
	dmg = wf_game.grenade_damage[GRENADE_TYPE_PROXIMITY];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_PROXIMITY;
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

	//What type of proximity?  1= grenade 2=pipe
	if (proximity_type == PROXIMITY_TYPE_GRENADE)
	{
		grenade->s.modelindex = gi.modelindex (GRPROXIMITY_MODEL);
		grenade->s.skinnum = GRPROXIMITY_SKIN;
	}
	else
	{
		grenade->s.modelindex = gi.modelindex (GRPIPEBOMB_MODEL);
		grenade->s.skinnum = GRPIPEBOMB_SKIN;
	}
	grenade->owner = self;
	grenade->touch = Proximity_Touch;

	grenade->nextthink = level.time + .1;
	grenade->think = proxim_think;
	grenade->delay = level.time + 120;
	grenade->delay2 = level.time + 1;		//don't exlode for at least 1 second

	grenade->dmg = dmg;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "proximity";
	grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	// CCH: a few more attributes to let the grenade 'die'
    VectorSet(grenade->mins, -3, -3, 0);
    VectorSet(grenade->maxs, 3, 3, 6);
    grenade->mass = 2;
    grenade->health = 10;
    grenade->die = Proximity_Die;
    grenade->takedamage = DAMAGE_YES;
    grenade->monsterinfo.aiflags = AI_NOSTEP;
	//set the team
	if ((int)wfflags->value & WF_ANARCHY)
		grenade->wf_team = 0;	//fire at anybody
	else
		grenade->wf_team = self->client->resp.ctf_team;



	if (timer <= 0.0)
		Proximity_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}
