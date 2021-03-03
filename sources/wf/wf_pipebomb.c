/*==============================================================================
The Weapons Factory - 
Pipe Bomb Functions
Original code by Chris Hilton
Modified by Gregg Reno
==============================================================================*/
#include "g_local.h"

//From Chris Hilton

void Pipebomb_Explode (edict_t *ent)
{
	vec3_t		origin;

//	gi.dprintf("Pipebomb Explode\n");

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_WF_PIPEBOMB);

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

void Pipebomb_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//gi.dprintf("Pipebomb TOUCH\n");

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

	//ent->enemy = other;
	Pipebomb_Explode (ent);
}

void Pipebomb_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Pipebomb_Explode;
}


void fire_pipebomb (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	++self->client->pers.active_grenades[GRENADE_TYPE_PIPEBOMB];

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_PIPEBOMB;
	grenade->wf_team = self->client->resp.ctf_team;

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
//	grenade->touch = Pipebomb_Touch;

	grenade->nextthink = level.time + 180;	//3 minutes
	grenade->think = Pipebomb_Explode;

	//grenade->delay = level.time + 60;
	grenade->dmg = wf_game.grenade_damage[GRENADE_TYPE_PIPEBOMB];
	grenade->dmg_radius = damage_radius;
	grenade->classname = "pipebomb";
	grenade->spawnflags = 1;

	//Make these silent!
//	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	// CCH: a few more attributes to let the grenade 'die'
    VectorSet(grenade->mins, -3, -3, 0);
    VectorSet(grenade->maxs, 3, 3, 6);
    grenade->mass = 2;
    grenade->health = 10;
    grenade->die = Pipebomb_Die;
    grenade->takedamage = DAMAGE_YES;
    grenade->monsterinfo.aiflags = AI_NOSTEP;
    
	if (timer <= 0.0)
	{
		//Pipebomb_Explode (grenade);
//		gi.dprintf("Pipebomb: Zero timer!\n");
	}
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

/*
=================
Cmd_DetPipes_f
CCH: new function to detonate all pipebombs within 1000 units
=================
*/
void Cmd_DetPipes_f (edict_t *ent)
{
	edict_t *blip = NULL;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!strcmp(blip->classname, "pipebomb") && blip->owner == ent)
		{
			blip->think = Pipebomb_Explode;
			blip->nextthink = level.time + .1;
		}
	}
}



