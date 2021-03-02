#include "../g_local.h"

void ThrowWater (edict_t *self, char *modelname, float speed, vec3_t origin, vec3_t aimdir);
static void Balloon_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;
	int u;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;
		int kick;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		mod = MOD_BALLOON;
		if (ent->enemy->groundentity)
			kick = 150;
		else
			kick = 500;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, kick, (DAMAGE_NO_ARMOR | DAMAGE_ENERGY), mod);
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	/*gi.WriteByte (svc_temp_entity);
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
	gi.multicast (ent->s.origin, MULTICAST_PHS);*/

	for (u = 0; u < 8; u++)
	{
		ThrowWater(ent, "sprites/null.sp2", 2, ent->s.origin, NULL);
	}
	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/waterballoon.wav"), 1, ATTN_NORM, 0);
	G_FreeEdict (ent);
}

void Balloon_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void BlowUp (edict_t *ent)
{
	ent->inair = 888;
	Balloon_Explode (ent);
}

void Balloon_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	//if (ent->velocity[PITCH] > 120)
	//	gi.dprintf ("Pitch < 60\n");
	//safe_cprintf (ent->owner, PRINT_HIGH, "%f\n", ent->velocity[PITCH]);
	//if (ent->velocity[ROLL] < -560)
		//safe_cprintf (ent->owner, PRINT_HIGH, "Pitch > -560\n");

	if (ent->inair <= 5 && ent->think != BlowUp)
	{
		if (other->takedamage)
		{
			ent->enemy = other;
			Balloon_Explode (ent);
		}
		else
		{
			ent->movetype = MOVETYPE_BOUNCE;
			//safe_cprintf (ent->owner, PRINT_HIGH, "In air 3 seconds!\n");
			ent->think2 = BlowUp;
			ent->nextthink2 = level.time + 2;
		}
		return;
	}
	else if (ent->think == BlowUp)
	{
		if (other->takedamage)
		{
			ent->enemy = other;
			Balloon_Explode (ent);
		}
		return;
	}

	ent->enemy = other;
	Balloon_Explode (ent);
}

void water_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	self->think = NULL;
	self->nextthink = 0;
	G_FreeEdict (self);
}

void SpawnSparkies (edict_t *self)
{
	vec3_t old;

	VectorCopy (self->tempvec, old);
//	G_Spawn_Sparks (TE_SHIELD_SPARKS, self->s.origin, self->velocity, self->s.origin);
	if (self->owner)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FORCEWALL);
		gi.WritePosition (old);
		gi.WritePosition (self->s.origin);
		gi.WriteByte (COLOR_BLUE);
		gi.multicast (old, MULTICAST_PHS);
	}
	else
		G_Spawn_Splash (TE_LASER_SPARKS, 20, COLOR_BLUE2, self->s.origin, self->velocity, self->s.origin);

	if (Q_stricmp(self->classname, "water") == 0)
	{
		if (self->velocity[0] == 0 && self->velocity[1] == 0 && self->velocity[2] == 0)
		{
			//gi.dprintf ("Freed sitting water\n");
			G_FreeEdict(self);
		}
	}

	self->nextthink = level.time + .1;
}

void water_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void InAir (edict_t *ent)
{
	ent->inair ++;

	ent->nextthink2 = level.time + 0.1;
}
void SaveOldOrigin (edict_t *self);

void ThrowWater (edict_t *self, char *modelname, float speed, vec3_t origin, vec3_t aimdir)
{
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	self->velocity[2] = 300;
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_TOSS;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	if (aimdir)
	{
		vec3_t	forward, right, up;
		vec3_t	dir;
		
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		//VectorClear (chunk->velocity);
		VectorClear (chunk->avelocity);
		VectorCopy (origin, chunk->s.origin);
		//VectorScale (aimdir, speed, chunk->velocity);
		//VectorMA (chunk->velocity, 200 + crandom() * 10.0, up, chunk->velocity);
		//VectorMA (chunk->velocity, crandom() * 10.0, right, chunk->velocity);
	}
	chunk->think = SpawnSparkies;
	chunk->nextthink = level.time + 0.1;
	chunk->think2 = G_FreeEdict;
	chunk->nextthink2 = level.time + 9;
	chunk->think3 = SaveOldOrigin;
	chunk->nextthink3 = level.time + 0.3;
	chunk->owner = self->owner;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "water";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = water_die;
	VectorCopy (chunk->s.origin, chunk->tempvec);
	gi.linkentity (chunk);
}

void SaveOldOrigin (edict_t *self)
{
	VectorCopy (self->s.origin, self->tempvec);
	self->nextthink3 = level.time + 0.1;
}

void fire_balloon (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
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
	grenade->movetype = MOVETYPE_TOSS;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = ModelIndex ("models/proj/balloon/tris.md2");
	grenade->owner = self;
	grenade->touch = Balloon_Touch;
	grenade->nextthink = level.time + 0.1;
	grenade->think = SpawnSparkies;
	grenade->think2 = InAir;
	grenade->nextthink2 = level.time + 0.1;
	grenade->think3 = SaveOldOrigin;
	grenade->nextthink3 = level.time + 0.1;
	grenade->dmg = damage;
	grenade->classname = "grenade";
	grenade->s.skinnum = rand()%6;
	if (grenade->s.skinnum == 6)
		grenade->s.skinnum = 0;
	grenade->can_teleport_through_destinations = 1;
	//gi.dprintf ("%i\n", grenade->s.skinnum);
	grenade->s.renderfx |= RF_IR_VISIBLE;
	VectorCopy (grenade->s.origin, grenade->tempvec);

	gi.linkentity (grenade);
}

void weapon_balloonlauncher_fire (edict_t *ent)
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

	fire_balloon (ent, start, forward, 50, 1000);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_BalloonLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_balloonlauncher_fire);
}
