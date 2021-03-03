#include "g_local.h"

/*
=====================
Pellet Missile
=====================
*/
void rocket_shot_think (edict_t *ent)
{
vec3_t		origin;
	//Sean added these 4 vectors	
	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;
	vec3_t   grenade5;
	vec3_t   grenade6;
	vec3_t   grenade7;
	vec3_t   grenade8;
	int		i;
	vec3_t offset;


	if (ent->owner->client)	
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	VectorSet(offset,0,0,32);
	VectorAdd(offset,ent->s.origin,offset);
	VectorCopy (offset, ent->s.origin);
	VectorSet(grenade1,20,20,0);
	VectorSet(grenade2,20,-20,0);
	VectorSet(grenade3,-20,20,0);
	VectorSet(grenade4,-20,-20,0);
	VectorSet(grenade5,0,20,0);
	VectorSet(grenade6,0,-20,0);
	VectorSet(grenade7,-20,0,0);
	VectorSet(grenade8,20,0,0);

	// Increased damage and decreased # bullets to reduce lag
	for (i = 0; i < 4; i++) 
//	for (i = 0; i < 15; i++) 
	{
	fire_bullet (ent, offset, grenade1, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade2, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade3, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade4, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade5, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade6, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade7, 12, 2, 15000, 15000,MOD_PELLET);
	fire_bullet (ent, offset, grenade8, 12, 2, 15000, 15000,MOD_PELLET);
	}

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_PELLET);

	VectorMA (ent->s.origin, -.02, ent->velocity, origin);
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
	// SumFuka did this bit : give grenades up/outwards velocities
	
	G_FreeEdict (ent);
}

void rocket_pellet_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;
	void rocket_shot_think (ent);

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0,MOD_PELLET);
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

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius,MOD_PELLET);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

void fire_pellet_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->creator = self;
	rocket->touch = rocket_pellet_touch;
	rocket->nextthink = level.time + .7;
	rocket->think = rocket_shot_think;
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
//	rocket->takedamage = DAMAGE_YES;
	rocket->takedamage = DAMAGE_NO;		//to fix self damage bug?
	rocket->monsterinfo.aiflags = AI_NOSTEP;
	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_PelletRocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;

	
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = wf_game.weapon_damage[WEAPON_PELLET];
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
	//First rocket
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_pellet_rocket (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_PELLET], damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_PelletRocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_PelletRocketLauncher_Fire);
}