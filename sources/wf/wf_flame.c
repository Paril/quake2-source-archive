/*
=================
Flame Grenade
=================
*/
#include "g_local.h"

void Flame_Explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//No damage for now
	//T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_WF_FLAME);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	TOTALWORLDFLAMES--;
	G_FreeEdict (ent);
//        gi.dprintf("Flame Explode\n");
}

void Flame_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	if (other == ent->owner)
//		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		TOTALWORLDFLAMES--;
		G_FreeEdict (ent);
		return;
	}

	//Attach to enemy if they touch the flame
	if (!ent->enemy)	//First time touch of an enemy?
	{
		//Only do this for certain kinds of entities
		if (other->client)	//is is a client?
		{
			ent->enemy = other;
		}
		else if (!strcmp(other->classname, "decoy"))
		{
			ent->enemy = other;
		}
		else if (!strcmp(other->classname, "player"))
		{
			ent->enemy = other;
		}

		//Stop the flames forward motion
		if (ent->enemy)
		{
			ent->movetype = MOVETYPE_NONE;
			VectorClear (ent->velocity);
			//burn em baby, dead people don't scream (TeT)
			if (ent->enemy->health > 0)
			{
				gi.sound (ent->enemy, CHAN_WEAPON, gi.soundindex ("scream.wav"), 1, ATTN_NORM, 0);
			}

		}
	}
}

void flame_think (edict_t *ent)
	{

	ent->nextthink = level.time + .2;
	ent->think = flame_think;

	//Exlode the flame if time has run out
    if (level.time > ent->delay)
	{
		Flame_Explode(ent);
		return;
	}

	//Exlode the flame if it's in the water
    if (ent->waterlevel)
	{
		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("world/airhiss1.wav"), 1, ATTN_NORM, 0);
		Flame_Explode(ent);
		return;
	}

	//If an enemy is defined, move the flame to it
	if (ent->enemy)
	{
		//If enemy is dead, destroy flame
		if (ent->enemy->deadflag == DEAD_DEAD)
		{
			Flame_Explode(ent);
		}
		else
		{

			//Move flame to where the entity is
			VectorCopy (ent->enemy->s.origin, ent->s.origin);

			//Cause some damage (increased from 3 to 4)
			T_Damage (ent->enemy, ent, ent->owner, ent->velocity, ent->s.origin, NULL, 4, 0, 0, MOD_WF_FLAME);

//			gi.dprintf("Flame Move\n");
		}
	}
}

void fire_flame (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*flame;
	vec3_t	dir;
	vec3_t	forward, right, up;

	if(TOTALWORLDFLAMES>=MAX_FLAMES)
	{
		if (!self->bot_client && self->client)//ERASER ACRID
			gi.bprintf (PRINT_HIGH, "WF WARNING!!! MAX FLAMES HAVE BEEN REACHED!\n");
		return;
	}
	TOTALWORLDFLAMES++;
	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	flame = G_Spawn();
	VectorCopy (start, flame->s.origin);
	VectorScale (aimdir, speed, flame->velocity);
	VectorMA (flame->velocity, 200 + crandom() * 10.0, up, flame->velocity);
	VectorMA (flame->velocity, crandom() * 10.0, right, flame->velocity);
	VectorSet (flame->avelocity, 300, 300, 300);

	flame->movetype = MOVETYPE_BOUNCE;
	flame->clipmask = MASK_SHOT;
	flame->solid = SOLID_BBOX;
//	flame->s.effects |= (int)(wftest->value) | EF_ANIM_ALLFAST;
	flame->s.effects |= EF_BFG | EF_HYPERBLASTER | EF_ANIM_ALLFAST;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

	flame->s.modelindex = gi.modelindex ("sprites/fire.sp2");

	flame->owner = self;
	flame->touch = Flame_Touch;

	flame->nextthink = level.time + .2;
	flame->think = flame_think;
	flame->delay = level.time + 6.0;	//stick around for this number of seconds

	flame->dmg = damage;
	flame->dmg_radius = damage_radius;
	flame->classname = "flame";
	flame->spawnflags = 1;
	//flame->s.sound = gi.soundindex("weapons/hgrenc1b.wav");
	flame->enemy = NULL;

	if (timer <= 0.0)
		Flame_Explode (flame);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (flame);
	}
}