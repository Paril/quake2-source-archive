#include "../g_local.h"

void NapalmStick(edict_t *ent, edict_t *other)
{
	if (ent->movetype == MOVETYPE_STUCK)
		return;
	
	ent->movetype = MOVETYPE_STUCK;
	VectorClear(ent->velocity);
	VectorClear(ent->avelocity);
	ent->stuckentity = other;
	VectorCopy(other->s.origin,ent->oldstuckorigin);
	VectorCopy(other->s.angles,ent->oldstuckangles);
	// Paril: Fixes the impeding movement and jump bug.
	ent->solid = SOLID_NOT;
}
void CreateStickingFire (edict_t *ent, vec3_t origin, edict_t *other);

void StickFire_Think (edict_t *ent)
{
	edict_t *blip = NULL;
	ent->s.frame++;

	if (ent->s.frame == 16)
		ent->s.frame = 6;

	if (!ent->stuckentity)
		NapalmStick (ent, ent->temp_other);

	if (!ent->stuckentity || !ent->stuckentity->fire_entity) // Whoops?
	{
		ent->stuckentity->fire_framenum = -1;
		ent->stuckentity->fire_hurtframenum = -1;
		G_FreeEdict (ent);
		return;
	}

	if (ent->stuckentity->waterlevel > 1) // In water.
	{
		G_Spawn_Splash (TE_TUNNEL_SPARKS, 36, 0x00000000, ent->s.origin, ent->s.origin, ent->s.origin);
		ent->stuckentity->fire_framenum = -1;
		ent->stuckentity->fire_hurtframenum = -1;
		G_FreeEdict (ent->stuckentity->fire_entity);
		G_FreeEdict (ent);
		return;
	}

	//G_Spawn_Splash (TE_TUNNEL_SPARKS, 8, COLOR_ORANGE, ent->s.origin, ent->s.origin, ent->s.origin);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_CHAINFIST_SMOKE);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (ent->stuckentity)
	{
		while (blip = (findradius(blip, ent->stuckentity->s.origin, 40)))
		{
			if (Q_stricmp(blip->classname, "water") == 0)
			{
				G_Spawn_Splash (TE_TUNNEL_SPARKS, 36, 0x00000000, ent->s.origin, ent->s.origin, ent->s.origin);
				ent->stuckentity->fire_framenum = -1;
				ent->stuckentity->fire_hurtframenum = -1;
				G_FreeEdict (ent->stuckentity->fire_entity);
				G_FreeEdict(ent);
				return;
			}
			if (blip == ent->stuckentity)
				continue;
			if (!blip->takedamage)
				continue;
			if (blip->solid != SOLID_BBOX)
				continue;
			if (blip->deadflag)
				continue;

//			gi.dprintf ("Catch?\n");

			if (random() < 0.2)
			{
				if (blip->fire_framenum && blip->fire_entity)
				{
					blip->fire_framenum += level.framenum + 3;
					continue;
				}
				// Fire stick!
				// Came from another person
				CreateStickingFire(ent, blip->s.origin, blip);
				blip->fire_framenum = level.framenum + 40;
			}

		}
	}

	ent->nextthink = level.time + .1;
}

void CreateStickingFire (edict_t *ent, vec3_t origin, edict_t *other)
{
	edict_t *fire;
	vec3_t angles = {0, 0, 0};

	if (!ent)
		return;

	fire = CreateEntity(origin, angles, MOVETYPE_NONE, 0, SOLID_NOT, 0, RF_TRANSLUCENT, 0, NULL, NULL, ent);
	fire->s.modelindex = ModelIndex ("models/other/fire/fire.md2");
	fire->s.frame = 6;
	fire->think = StickFire_Think;
	fire->nextthink = level.time + .1;
	fire->s.effects |= EF_HYPERBLASTER;
	// Direct fire from owner
	if (ent && Q_stricmp(ent->classname, "player") == 0)
		fire->owner = ent;
	// Direct fire to enemy
	else if (ent->owner && Q_stricmp(ent->owner->classname, "player") == 0)
		fire->owner = ent->owner;
	// No idea, shouldn't get here!
	else
	{
		gi.dprintf ("CreateStickingFire: No player owner found!\n");
		fire->owner = ent;
	}
	fire->temp_other = other;

	gi.linkentity (fire);

	other->fire_entity = fire;
	other->fire_hurtframenum = level.framenum + 10;
}
void SpreadFire (edict_t *self, float speed, vec3_t origin, int fracted);

void FrameUp (edict_t *ent)
{
	ent->s.frame++;

	if (ent->s.frame == 6)
		ent->s.frame = 0;

	//G_Spawn_Splash (TE_TUNNEL_SPARKS, 8, COLOR_ORANGE, ent->s.origin, ent->s.origin, ent->s.origin);

	// Fractalized fire has lower chance to spread.
	if (!ent->fract)
	{
		if (random() < 0.007)
		{
			int i;

			for (i = 0; i < 1+rand()%4; i++)
			{
				SpreadFire(ent->owner, 2+rand()%5, ent->s.origin, 1);
			}
		}
	}
	else
	{
		if (random() < 0.004)
		{
			int i;

			for (i = 0; i < 1+rand()%2; i++)
			{
				SpreadFire(ent->owner, 2+rand()%5, ent->s.origin, 1);
			}
		}
	}

	ent->nextthink2 = level.time + .1;
}

void Fire_HurtNearBy_Slowly (edict_t *ent)
{
	edict_t *blip = NULL;

	if (ent->waterlevel) // In water.
	{
		G_Spawn_Splash (TE_TUNNEL_SPARKS, 36, 0x00000000, ent->s.origin, ent->s.origin, ent->s.origin);
		G_FreeEdict (ent);
		return;
	}

	while (blip = findradius(blip, ent->s.origin, 70))
	{
		// First, check for around fire, and if any, disappear!
		if (Q_stricmp(blip->classname, "water") == 0)
		{
			G_Spawn_Splash (TE_TUNNEL_SPARKS, 36, 0x00000000, ent->s.origin, ent->s.origin, ent->s.origin);
			G_FreeEdict(ent);
			return;
		}
		if (!blip->inuse)
		{
			continue;
		}
		if (!blip->takedamage)
		{
			continue;
		}
		if (blip->health <= 0)
		{
			continue;
		}

		T_Damage (blip, ent, ent->owner, vec3_origin, blip->s.origin, vec3_origin, 4+rand()%8, 0, DAMAGE_NO_ARMOR, MOD_FIRESMALL); // Paril MOD FIXME
	
		// Is blip->client needed? Might
		// make specialized code for monsters...
		if (random() < 0.3)
		{
			if (blip->fire_framenum && blip->fire_entity)
			{
				blip->fire_framenum += level.framenum + 3;
				continue;
			}
			// Fire stick!
			CreateStickingFire(ent->owner, blip->s.origin, blip);
			blip->fire_framenum = level.framenum + 40;
		}
	}

	ent->nextthink = level.time + .8;
}

void SpreadFire (edict_t *self, float speed, vec3_t origin, int fracted)
{
	edict_t	*chunk;
	vec3_t	v;
	int i, all = 0;
	edict_t *cur;

	for (i=1, cur=g_edicts+i; i < globals.num_edicts; i++,cur++)
	{
		if (!cur->inuse)
			continue;
		if (cur->client)
			continue;
		if (!cur->classname)
			continue;

		if (Q_stricmp(cur->classname, "fire") == 0)
			all++;
	}

	if (all > 59)
		return;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	chunk->s.modelindex = ModelIndex ("models/other/fire/fire.md2");
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_TOSS;
	chunk->solid = SOLID_NOT;
	//VectorSet (chunk->mins, -6, -6, -10);
//	VectorSet (chunk->maxs, 6, 6, 10);
	//chunk->avelocity[0] = random()*600;
	//chunk->avelocity[1] = random()*600;
	//chunk->avelocity[2] = random()*600;
	chunk->think = Fire_HurtNearBy_Slowly;
	chunk->nextthink = level.time + .1;
	chunk->think2 = FrameUp;
	chunk->nextthink2 = level.time + .1;
	chunk->think3 = G_FreeEdict;
	chunk->nextthink3 = level.time + 6+rand()%3;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "fire";
	chunk->owner = self;
	chunk->fract = fracted;
	chunk->s.effects |= EF_HYPERBLASTER;
	gi.linkentity (chunk);
}

static void NGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

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
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	for (mod = 0; mod < 5+rand()%5; mod ++)
	{
		SpreadFire (ent->owner, 0+rand()%5, ent->s.origin, 0);
	}

	G_FreeEdict (ent);
}

void NGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		NGrenade_Explode (ent);
		return;
	}

	ent->enemy = other;
	NGrenade_Explode (ent);
}

void fire_ngrenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer)
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
	grenade->s.effects |= EF_GRENADE | EF_FLAG1 | EF_COLOR_SHELL;
	grenade->s.renderfx |= RF_SHELL_RED;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = ModelIndex ("models/proj/incend/tris.md2");
	grenade->owner = self;
	grenade->touch = NGrenade_Touch;
	//grenade->nextthink = level.time + timer;
	//grenade->think = NGrenade_Explode;
	grenade->classname = "ngrenade";
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;
//	grenade->s.frame = grenade->s.skinnum = 5;

	gi.linkentity (grenade);
}


static void Bean_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

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
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	for (mod = 0; mod < 2+rand()%2; mod ++)
	{
		SpreadFire (ent->owner, 0+rand()%5, ent->s.origin, 0);
	}

	G_FreeEdict (ent);
}
void DrunkHit (edict_t *drinker);

void Bean_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int mod;
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		//if (random() < 0.5)
		//{
			for (mod = 0; mod < 2+rand()%2; mod ++)
			{
				SpreadFire (ent->owner, 0+rand()%5, ent->s.origin, 0);
			}
		//}
		DrunkHit(ent);

		return;
	}

	ent->enemy = other;
	Bean_Explode (ent);
}

void fire_bean (edict_t *self, vec3_t start, vec3_t aimdir, int speed)
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
	grenade->movetype = MOVETYPE_BOUNCENOSTOP;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = ModelIndex ("models/proj/bean/tris.md2");
	grenade->owner = self;
	grenade->touch = Bean_Touch;
	grenade->nextthink = level.time + 14;
	grenade->think = G_FreeEdict;
	grenade->classname = "bean";
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;

	gi.linkentity (grenade);
}


void Weapon_NapalmGrenadeLauncher_Fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_ngrenade (ent, start, forward, 1500, 2.5);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_NapalmGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, Weapon_NapalmGrenadeLauncher_Fire);
}

void Weapon_Bean_Fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_bean (ent, start, forward, 1000);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Bean (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, Weapon_Bean_Fire);
}
