#include "../g_local.h"

void Shard_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
//	int			n;

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
	VectorMA (ent->s.origin, -0.0/*2*/, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_SHARD);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLECHETTE);
	gi.WritePosition (origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

//	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void fire_shard (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
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
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/shard/tris.md2");
	rocket->owner = self;
	rocket->touch = Shard_Touch;
	rocket->nextthink = level.time + 8000;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->s.renderfx |= RF_TRANSLUCENT;
//	rocket->radius_dmg = radius_damage;
//	rocket->dmg_radius = damage_radius;
	rocket->classname = "shard";
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;
	//rocket->s.frame = rocket->s.skinnum = 3;
	rocket->s.effects |= EF_COLOR_SHELL;
	rocket->s.renderfx |= RF_SHELL_GREEN|RF_SHELL_BLUE;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_ShardCannon_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int damage;
	int i;

	damage = 12;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
	}

	/*v[PITCH] = ent->client->v_angle[PITCH] - rand()%7 + rand()%7;
	v[YAW]   = ent->client->v_angle[YAW] - rand()%7 + rand()%7;
	v[ROLL]  = ent->client->v_angle[ROLL] - rand()%7 + rand()%7;
	AngleVectors (v, forward, NULL, NULL);
	fire_shard (ent, start, forward, 8, 950);*/
	for (i = 0; i < 8+rand()%6; i++)
	{
		v[PITCH] = ent->client->v_angle[PITCH] - random()*5 + random()*5;
		v[YAW]   = ent->client->v_angle[YAW] - random()*5 + random()*5;
		v[ROLL]  = ent->client->v_angle[ROLL] - random()*5 + random()*5;
		AngleVectors (v, forward, NULL, NULL);
		fire_shard (ent, start, forward, damage, 1550);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] --;
}

void Weapon_ShardCannon (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, Weapon_ShardCannon_Fire);
}
