#include "g_local.h"
/*
=========================
Sentry Killer Rocket
=========================
*/

// WF & CCH: New think function for homing missiles
void sentryhoming_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t  targetdir, blipdir;
	vec_t   speed;

//gi.dprintf("Sentry Killer Think\n");

	while ((blip = findradius(blip, ent->s.origin, 1500)) != NULL)
	{

		//The only non-client to track is sentry guns
		if ((strcmp(blip->classname, "SentryGun") != 0))
			continue;
		if (blip == ent->owner)
			continue;
		//dont aim at same team unless friendly fire is on
  	    if ((blip->wf_team == ent->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
		//if (blip->wf_team == ent->wf_team)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}
                    
	if (target != NULL)
	{
		 // target acquired, nudge our direction toward it
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.2, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);

		//is this the first time we locked in? sound warning for the target
		if (ent->homing_lock == 0)
		{
			gi.sound (target, CHAN_AUTO, gi.soundindex ("homelock.wav"), 1, ATTN_NORM, 0);
			ent->homing_lock = 1;
		}
	}
     
	ent->nextthink = level.time + .1;
	ent->think = sentryhoming_think;
}

void sentryrocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_sentryrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");
	rocket->owner = self;
	rocket->touch = sentryrocket_touch;

//WF & CCH - Add homing lock status
	rocket->homing_lock = 0;

	//Set the team of the rocket
	rocket->wf_team = self->wf_team;
	
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;

	// See if this is a player and if they have homing on
	rocket->die = Rocket_Die;
	rocket->health = 10;
	// CCH: a few more attributes to let the rocket 'die'
	VectorSet(rocket->mins, -10, -3, 0);
	VectorSet(rocket->maxs, 10, 3, 6);
	rocket->mass = 10;
	rocket->nextthink = level.time + .1;
	rocket->think = sentryhoming_think;

    //reduce damage of homing rockets to 1/2 of normal rocket
    rocket->dmg = damage;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_SentryRocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = wf_game.weapon_damage[WEAPON_SENTRYKILLER];    //was 150 G.R.
	radius_damage = 100;	//was 120 G.R.
	damage_radius = 35;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//WF - increase speed of rocket
//	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
	fire_sentryrocket (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_SENTRYKILLER] , damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]-= 10;
}

void Weapon_SentryRocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_SentryRocketLauncher_Fire);
}

