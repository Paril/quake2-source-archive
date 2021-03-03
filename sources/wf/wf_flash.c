/*==============================================================================
The Weapons Factory - 
Flash Grenade Functions
Original code by ??
Modified by Gregg Reno
==============================================================================*/
#include "g_local.h"
#define FLASH_RADIUS	300
#define BLIND_FLASH		50      // Time of blindness in FRAMES


void Flash_Explode (edict_t *ent)
{
	vec3_t      offset, v;
	edict_t *target;
    float Distance, BlindTimeAdd;
//	vec3_t      origin;

	//safe_cprintf(ent->owner, PRINT_HIGH, "Flash goes BOOM!\n");

	// Move it off the ground so people are sure to see it
	VectorSet(offset, 0, 0, 10);    
	VectorAdd(ent->s.origin, offset, ent->s.origin);

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	target = NULL;
	while ((target = findradius(target, ent->s.origin, FLASH_RADIUS)) != NULL)
	{
//		if (target == ent->owner)
//			continue;       // You know when to close your eyes, don't you?
		if (!target->client)
			continue;       // It's not a player
		if (!visible(ent, target))
			continue;       // The grenade can't see it
//		if (!infront(target, ent))
//			continue;       // It's not facing it

		//dont blind teammates
		if (target->wf_team == ent->wf_team)
			  continue;

		// Find distance
		VectorSubtract(ent->s.origin, target->s.origin, v);
		Distance = VectorLength(v);

		// Calculate blindness factor
		if ( Distance < FLASH_RADIUS/10 )
			BlindTimeAdd = BLIND_FLASH; // Blind completely
		else
			BlindTimeAdd = 1.5 * BLIND_FLASH * ( 1 / ( ( Distance - FLASH_RADIUS*2 ) / (FLASH_RADIUS*2) - 2 ) + 1 ); // Blind partially
		if ( BlindTimeAdd < 0 )
			BlindTimeAdd = 0; // Do not blind at all.

		// Not facing it, but still blinded a little
		if (!infront(target, ent))
			BlindTimeAdd *= .5;
		// You know when to close your eyes, don't you? Doesn't quite do the job. :)
		if (target == ent->owner)
		{
			target->client->blindTime += BlindTimeAdd * .3;
			target->client->blindBase = BLIND_FLASH;
			continue;
		}

		// Increment the blindness counter
		target->client->blindTime += BlindTimeAdd * 1.5;
//
//target->client->blindTime = 300;
//
		target->client->blindBase = BLIND_FLASH;
		target->s.angles[YAW] = (rand() % 360);  // Whee!

		// Let the player know what just happened
		// (It's just as well, he won't see the message immediately!)
		safe_cprintf(target, PRINT_HIGH, "You are blinded by a flash grenade!!!\n");

		// Let the owner of the grenade know it worked
		safe_cprintf(ent->owner, PRINT_HIGH, "%s is blinded by your flash grenade!\n",
			target->client->pers.netname);
	}

	// Blow up the grenade
	BecomeExplosion1(ent);
}


void Flash_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	// Dont blow up if on same team
	if (other->wf_team == ent->wf_team)
		return;

	// If it goes in to orbit, it's gone...
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// All this does is make the bouncing noises when it hits something...
	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"),
					1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"),
					1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex("weapons/grenlb1b.wav"),
				1, ATTN_NORM, 0);
		}
        return;
    }

    // The ONLY DIFFERENCE between this and "Grenade_Touch"!!
    Flash_Explode (ent);    
}



void fire_flash (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->wf_team = self->wf_team;
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
//	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->s.modelindex = gi.modelindex (GRFLASH_MODEL);
	grenade->s.skinnum = GRFLASH_SKIN;
	grenade->owner = self;
    grenade->touch = Flash_Touch;
    grenade->think = Flash_Explode;
    grenade->classname = "flash_grenade";
	grenade->nextthink = level.time + timer;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	//safe_cprintf(self, PRINT_HIGH, "FLASH!\n");

	if (timer <= 0.0)
		GenericGrenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}
