/*==============================================================================
The Weapons Factory -
Goodyear Grenade Functions
Original code by Gregg Reno
==============================================================================*/
/*
   This grendade is launched like a ballon and explodes like
   a proximity bomb.
*/

#include "g_local.h"

/*
=================
fire_goodyear
=================
*/

void Goodyear_Explode (edict_t *ent)
{
	vec3_t		origin;


	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_WF_GOODYEAR);

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

// When a grenade 'dies', it blows up next frame
void Goodyear_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Goodyear_Explode;
}
void Goodyear_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//if (other == ent->owner)
	//	return;

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

	//ent->enemy = other;
	Goodyear_Explode (ent);
}


// New think function for goodyear grenades
void goodyear_think (edict_t *ent)
{
    edict_t *blip = NULL;

    if (level.time > ent->delay)
    {
        Goodyear_Explode(ent);
        return;
    }

	//is it armed yet?
    if (level.time < ent->delay2)
	{
	    ent->nextthink = level.time + .1;
		return;
	}

	ent->think = goodyear_think;
    while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
	{
        if (!(blip->svflags & SVF_MONSTER) && !blip->client)
            continue;
        //if (blip == ent->owner)
        //    continue;
 	    //dont attack same team
		if (blip->wf_team == ent->wf_team)
		    continue;
		if (blip->disguised)
			continue;
        if (!blip->takedamage)
            continue;
        if (blip->health <= 0)
            continue;
        if (!visible(ent, blip))
            continue;
        ent->think = Goodyear_Explode;
        break;
    }

    ent->nextthink = level.time + .1;
}


void fire_goodyear (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*goodyear;
	vec3_t	dir;
	vec3_t	forward, right, up;
	int dmg;

	++self->client->pers.active_grenades[GRENADE_TYPE_GOODYEAR];

	//reduce damage
	dmg = wf_game.grenade_damage[GRENADE_TYPE_GOODYEAR];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	goodyear = G_Spawn();
	VectorCopy (start, goodyear->s.origin);
	vectoangles (aimdir, goodyear->s.angles);
	VectorScale (aimdir, speed, goodyear->velocity);

	VectorSet (goodyear->avelocity, 300, 300, 300);

	goodyear->grenade_index = GRENADE_TYPE_GOODYEAR;
	goodyear->movetype = MOVETYPE_FLOAT;
	goodyear->clipmask = MASK_SHOT;
	goodyear->solid = SOLID_BBOX;
	goodyear->s.effects |= EF_GRENADE;
	VectorClear (goodyear->mins);
	VectorClear (goodyear->maxs);
//    VectorSet(goodyear->mins, -10, -10, 0);
//    VectorSet(goodyear->maxs, 10, 10, 10);
	goodyear->s.modelindex = gi.modelindex (GRGOODYEAR_MODEL);
	goodyear->s.skinnum = GRGOODYEAR_SKIN;

	goodyear->owner = self;
	goodyear->touch = Goodyear_Touch;

	goodyear->nextthink = level.time + .1;
	goodyear->think = goodyear_think;
	goodyear->delay = level.time + 120;
	goodyear->delay2 = level.time + 1;		//don't exlode for at least 1 second

	goodyear->dmg = dmg;
	goodyear->dmg_radius = damage_radius;
	goodyear->classname = "goodyear";

    goodyear->die = Goodyear_Die;
    goodyear->takedamage = DAMAGE_YES;
    //goodyear->takedamage = DAMAGE_NO;
	goodyear->spawnflags = 1;
	goodyear->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	goodyear->mass = 2;
	goodyear->health = 20;
    goodyear->max_health = 20;

	//set the team
	if ((int)wfflags->value & WF_ANARCHY)
		goodyear->wf_team = 0;	//fire at anybody
	else
		goodyear->wf_team = self->client->resp.ctf_team;

	gi.linkentity (goodyear);
}
