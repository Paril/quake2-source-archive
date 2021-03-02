#include "../g_local.h"

void Gas_Hurt (edict_t *ent)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, ent->s.origin, 280))
	{
		if (!blip->inuse)
			continue;
		if (!blip->takedamage)
			continue;

		T_Damage (blip, ent, ent->owner, vec3_origin, blip->s.origin, vec3_origin, 7, 0, 0, MOD_GAS); // FIXME mod
	}
	ent->s.frame++;
	if (ent->s.frame >= 7)
		ent->s.renderfx |= RF_TRANSLUCENT;
	if (ent->s.frame >= 10)
		ent->s.effects |= EF_SPHERETRANS;
	if (ent->s.frame == 15)
	{
		G_FreeEdict(ent);
		return;
	}
	
	ent->nextthink = level.time + .1;
}

void FireGasCloud (edict_t *ent)
{
	edict_t *cloud;
	vec3_t angles = {0, 0, 0};

	cloud = CreateEntity(ent->s.origin, angles, MOVETYPE_NONE, 0, SOLID_NOT, 0, 0, 0, NULL, NULL, ent->owner);

	cloud->s.modelindex = ModelIndex ("models/other/gascloud/tris.md2");
	cloud->think = Gas_Hurt;
	cloud->nextthink = level.time + .1;

	gi.linkentity (cloud);
}

// Toxic Gas Gun
static void GasGun_Explode (edict_t *ent)
{
//	vec3_t		origin;
//	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	FireGasCloud(ent);

	if (ent->tempint == 2)
		G_FreeEdict(ent);

	ent->tempint++;

	ent->nextthink = level.time + 2.0;
	ent->think = GasGun_Explode;
}

void GasGun_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
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
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
}

void fire_gasgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_SPINNINGLIGHTS;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = ModelIndex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = GasGun_Touch;
	grenade->nextthink = level.time + 1.0;
	grenade->think = GasGun_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	grenade->tempint = 0;
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}

void Weapon_GasGun_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 6;
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_gasgun (ent, start, forward, 0, 1000, 2.5, 0, false);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_GasGun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, Weapon_GasGun_Fire);
}
