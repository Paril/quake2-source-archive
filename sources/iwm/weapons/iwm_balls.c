// Rubber Ball
/*
=================
fire_grenade
=================
*/
#include "../g_local.h"

static void RubberExplode (edict_t *ent)
{
//	vec3_t		origin;
//	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);

		if (ent->enemy->die)
			T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, 15, 0, DAMAGE_NO_ARMOR|DAMAGE_ENERGY, MOD_RUBBERBALL);
	}

	//G_FreeEdict (ent);
}

void DrunkHit (edict_t *drinker)
{
	int p = (rand() % 360);
	int o = (rand() % 360)/*(rand() % 360)*/;
	int i = (rand() % 360);
	int p2 = (rand() % 360);
	int o2 = (rand() % 360)/*(rand() % 360)*/;
	int i2 = (rand() % 360);

	drinker->s.angles[PITCH] += p -= o2;
	drinker->s.angles[YAW] += o -= i2;
	drinker->s.angles[ROLL] += i -= p2;

	drinker->velocity[PITCH] += p -= o2;
	drinker->velocity[YAW] += o -= i2;
	drinker->velocity[ROLL] += i -= p2;

	//drinker->nextthink = level.time + 0.1;
}

static void Rubber_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	DrunkHit (ent);

	//if (surf && (surf->flags & SURF_SKY))
	//{
	//	G_FreeEdict (ent);
	//	return;
	//}

	if (!other->takedamage)
	{
		gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/rballbounce.wav"), 1, ATTN_NORM, 0);
		return;
	}

	ent->enemy = other;
	RubberExplode (ent);
}

void fire_rubberball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
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
	grenade->movetype = MOVETYPE_RUBBER;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	//VectorClear (grenade->mins);
	//VectorClear (grenade->maxs);
	VectorSet (grenade->mins, -10, -10, -10);
	VectorSet (grenade->maxs, 10, 10, 10);
	grenade->s.modelindex = ModelIndex ("models/proj/rball/tris.md2");
	grenade->owner = self;
	grenade->touch = Rubber_Touch;
	grenade->nextthink = level.time + 10;
	grenade->think = G_FreeEdict;
	grenade->dmg = damage;
//	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;

	gi.linkentity (grenade);
}

/*
=================
fire_grenade
=================
*/
static void Basketball_Explode (edict_t *ent)
{
//	vec3_t		origin;
//	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);

		if (ent->enemy->die)
			T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, 15, 0, DAMAGE_NO_ARMOR|DAMAGE_ENERGY, MOD_BASKETBALL);
	}

	//G_FreeEdict (ent);
}

void BballBounce (edict_t *drinker)
{
	int o = (rand() % 6);
	int i = (rand() % 6);

	drinker->s.angles[YAW] += o;
	drinker->s.angles[ROLL] += i;

	drinker->velocity[YAW] += o;
	drinker->velocity[ROLL] += i;

	//drinker->nextthink = level.time + 0.1;
}

void BballRaiseAndThrow (edict_t *drinker)
{
	vec3_t	kvel;
	float	mass;
	
	if (drinker->mass < 50)
		mass = 50;
	else
		mass = drinker->mass;
	
	VectorScale (drinker->velocity, 200.0 / mass, kvel);
	
	VectorAdd (drinker->velocity, kvel, drinker->velocity);
}

static void Basketball_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
	{
		BballRaiseAndThrow(ent);
		return;
	}

	BballBounce (ent);

	//if (surf && (surf->flags & SURF_SKY))
	//{
	//	G_FreeEdict (ent);
	//	return;
	//}

	if (!other->takedamage)
	{
		gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/bounce.wav"), 1, ATTN_NORM, 0);
		
		if (Q_stricmp(other->classname, "worldspawn") == 0)
			return;
		
		BballRaiseAndThrow(ent);
		return;
	}
	else if (other->takedamage && !ent->avelocity[1])
		BballRaiseAndThrow (ent);

	if (!ent->avelocity[1])
	{
		return;
	}
	ent->enemy = other;
	Basketball_Explode (ent);
}

void fire_basketball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
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
	//VectorClear (grenade->mins);
	//VectorClear (grenade->maxs);
	VectorSet (grenade->mins, -10, -10, -10);
	VectorSet (grenade->maxs, 10, 10, 10);
	grenade->s.modelindex = ModelIndex ("models/proj/bball/tris.md2");
	grenade->owner = self;
	grenade->touch = Basketball_Touch;
	grenade->nextthink = level.time + 15;
	grenade->think = G_FreeEdict;
	grenade->dmg = damage;
//	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;


	gi.linkentity (grenade);
}




/*
=================
fire_grenade
=================
*/
static void Beachball_Explode (edict_t *ent)
{
//	vec3_t		origin;
//	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);

		if (ent->enemy->die)
			T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, 45, 0, DAMAGE_NO_ARMOR|DAMAGE_ENERGY, MOD_BEACHBALL);
	}

	//G_FreeEdict (ent);
}

static void Beachball_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
	{
		BballRaiseAndThrow(ent);
		return;
	}

	BballBounce (ent);

	//if (surf && (surf->flags & SURF_SKY))
	//{
	//	G_FreeEdict (ent);
	//	return;
	//}

	if (!other->takedamage)
	{
		gi.sound (ent, CHAN_VOICE, SoundIndex ("weapons/bounce.wav"), 1, ATTN_NORM, 0);
		
		if (Q_stricmp(other->classname, "worldspawn") == 0)
			return;
		
		//BballRaiseAndThrow(ent);
		return;
	}
	else if (other->takedamage && !ent->avelocity[1])
		BballRaiseAndThrow (ent);

	if (!ent->avelocity[1])
	{
		return;
	}

	ent->enemy = other;
	Beachball_Explode (ent);
}

void fire_beachball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
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
	//VectorClear (grenade->mins);
	//VectorClear (grenade->maxs);
	VectorSet (grenade->mins, -18, -18, -18);
	VectorSet (grenade->maxs, 18, 18, 18);
	grenade->s.modelindex = ModelIndex ("models/proj/beachball/tris.md2");
	grenade->owner = self;
	grenade->touch = Beachball_Touch;
	grenade->nextthink = level.time + 15;
	grenade->think = G_FreeEdict;
	grenade->dmg = damage;
//	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	grenade->gravity = 0.250;
	grenade->can_teleport_through_destinations = 1;
	grenade->s.renderfx |= RF_IR_VISIBLE;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}


void weapon_rubberlauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 20;
//	float	radius;

	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->pers.weapon == FindItem ("Basketball Launcher"))
		fire_basketball (ent, start, forward, damage, 800);
	else if (ent->client->pers.weapon == FindItem ("Beach Ball Launcher"))
		fire_beachball (ent, start, forward, damage, 500);
	else
		fire_rubberball (ent, start, forward, damage, 1300);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_BallLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34-6, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16-6, 59, 64, pause_frames, fire_frames, weapon_rubberlauncher_fire);
}
