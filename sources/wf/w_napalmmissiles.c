#include "g_local.h"
/*
======================
Napalm Missiles
======================
*/

// Napalm rockets will home in on the laser target if it exists
void rocketnapalm_think (edict_t *ent)
{
	edict_t *target = NULL;
	vec3_t  targetdir;
	vec_t   speed;

	if ((ent->owner) && (ent->owner->client->pers.homing_state == 0))
	{
		ent->nextthink = level.time + .3;
		return;
	}

	if ((ent->owner) && (ent->owner->lasersight))
	{
		target = ent->owner->lasersight;
		VectorSubtract(target->s.origin, ent->s.origin, targetdir);
		targetdir[2] += 16;
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.135, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);
	}

	ent->nextthink = level.time + .1;
}

void rocketnapalm_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int		i;	
//	vec3_t	org;	
//	float		spd;

	vec3_t		origin;
	int			n;
    edict_t *blip = NULL;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0,MOD_NAPALMROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius,MOD_NAPALMROCKET);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//See if there is anything to catch on fire
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

		burn_person(blip, ent->owner, 6, MOD_NAPALMROCKET);
        break;
        }
     
	T_ShockWave(ent, 255, 1024);
	BecomeNewExplosion (ent);
}

void fire_napalmrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	rocket->wf_team = self->wf_team;
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	rocket->s.effects |= EF_BFG | EF_HYPERBLASTER;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocketnapalm_touch;
//	rocket->nextthink = level.time + 8000/speed;
//	rocket->think = G_FreeEdict;
	rocket->nextthink = level.time + .1;
	rocket->think = rocketnapalm_think;

	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	// CCH: a few more attributes to let the rocket 'die'
	VectorSet(rocket->mins, -10, -3, 0);
	VectorSet(rocket->maxs, 10, 3, 6);
	rocket->mass = 10;+	rocket->health = 10;
	rocket->die = Rocket_Die;
	rocket->takedamage = DAMAGE_NO;
	rocket->monsterinfo.aiflags = AI_NOSTEP;
	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_RocketNapalmLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;

	
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = wf_game.weapon_damage[WEAPON_NAPALMMISSLE];
	radius_damage = 20;
	damage_radius = 20;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;
	//First rocket
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_napalmrocket (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_NAPALMMISSLE], damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
//	ent->client->pers.inventory[ent->client->ammo_index] -= 2;	//it costs 2 rockets
}
void Weapon_RocketNapalmLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketNapalmLauncher_Fire);
}