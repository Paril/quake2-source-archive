#include "g_local.h"

/*
======================================================================

Stinger Launcher
by Aleksey 'Fireball' Bragin, 08/12/1998
Modified by Gregg Reno 12/24/98

======================================================================
*/

void Rocket_Explode (edict_t *ent);

//If stinger touches anything, just go dead
void stinger_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	ent->s.sound = 0;
	ent->movetype = MOVETYPE_BOUNCE;
	ent->delay = level.time + 3;

	//Play an impact sound
	//gi.sound (ent, CHAN_WEAPON, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex ("darthit.wav"), 1, ATTN_NORM, 0);

}

void stinger_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t  targetdir, blipdir;
	vec_t   speed;
	float	dist;

	//Blow up if we are past the delay time
	if (level.time > ent->delay)
	{
		ent->nextthink = level.time + .1;
		ent->think = Rocket_Explode;
		return;
	}

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!blip->client)
			continue;
		if (blip == ent->creator)
			continue;
		if (blip->disguised)
			continue;
		//dont aim at same team unless friendly fire is on
  	    if ((blip->wf_team == ent->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
			continue;
//		if (!blip->takedamage)
//			continue;
		if (blip->health <= 0)
			continue;
		if (!visible(ent, blip))
			continue;
		if (!infront(ent, blip))
			continue;
		
		if ((!blip->client->thrusting) && (!blip->client->ctf_grapple))
			continue;

		//We have a possible target.  If we are close
		//enough, just blow up.
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);

		if (dist < 100) 
		{
			ent->nextthink = level.time + .1;
			ent->think = Rocket_Explode;
			return;
		}

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
		VectorScale(targetdir, 0.28, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);

		//is this the first time we locked in? sound warning for the target
		if (ent->homing_lock == 0)
		{
//			gi.sound (target, CHAN_AUTO, gi.soundindex ("homelock.wav"), 1, ATTN_NORM, 0);
//			gi.sound (target, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
			ent->homing_lock = 1;
		}
	}
     
	ent->nextthink = level.time + .1;
}

void fire_stinger (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->creator = self;
	rocket->mod = MOD_STINGER;
	rocket->touch = stinger_touch;

	//Set the team of the rocket
	rocket->wf_team = self->wf_team;
	
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;

	rocket->nextthink = level.time + .1;
	rocket->think = stinger_think;

    rocket->dmg = damage;

	rocket->delay = level.time + 10;	//blow up after 10 seconds no mater what

	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	// Don't forget to set owner
	rocket->owner = self;
	gi.linkentity (rocket);
}

void Weapon_StingerLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = wf_game.weapon_damage[WEAPON_STINGER];
	radius_damage = 120;
	damage_radius = 120;

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
	fire_stinger (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_STINGER], damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -=5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
		ent->client->pers.inventory[ent->client->ammo_index] = 0;
}

void Weapon_StingerLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {7, 0};
	static int	fire_frames[] = {2, 0};

	Weapon_Generic (ent, 1, 6, 8, 10, pause_frames, fire_frames, Weapon_StingerLauncher_Fire);
}
