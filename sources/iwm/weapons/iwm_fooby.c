#include "../g_local.h"

void TWGDie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowWatermelonGib (edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
//	gi.setmodel (chunk, modelname);
	chunk->s.modelindex = ModelIndex(modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "gib";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = TWGDie;
	level_gibs++;

	gi.linkentity (chunk);
}

void Fooby_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	//if (surf && (surf->flags & SURF_SKY))
	//{
	//	G_FreeEdict (ent);
	//	return;
	//}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, MOD_FOOBY);
	}

	// Paril, SPLATWATERMELONHOLYCRAP.
	for (n = 0; n < 8; n++)
	{
		int r = rand() % 2;
		
		if (r == 0)
			ThrowWatermelonGib (ent, "models/proj/watermelon/gib1.md2", 2 + rand()%2, ent->s.origin);
		else if (r == 1)
			ThrowWatermelonGib (ent, "models/proj/watermelon/gib2.md2", 2 + rand()%2, ent->s.origin);
		else
			ThrowWatermelonGib (ent, "models/proj/watermelon/gib3.md2", 2 + rand()%2, ent->s.origin);
	}

	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/watermelonsplat.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void fire_fooby (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*rocket;

	damage *= 8;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->tempspeed = speed;
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/watermelon/tris.md2");
	rocket->owner = self;
	rocket->touch = Fooby_Touch;
	rocket->nextthink = level.time + 2000;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
//	rocket->radius_dmg = radius_damage;
//	rocket->dmg_radius = damage_radius;
	rocket->s.sound = SoundIndex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_Fooby_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
//	float	damage_radius;
//	int		radius_damage;
	int h;

	h = ent->client->ps.gunframe;

	damage = 100 + (int)(random() * 20.0);

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	if (h == 11)
		fire_fooby (ent, start, forward, damage, 600);
	else
		gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/dodododoledowheee.wav"), 1, ATTN_NORM, 0);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_ROCKET | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Fooby (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 11, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_Fooby_Fire);
}
