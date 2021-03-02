#include "../g_local.h"
void Stick(edict_t *ent, edict_t *other);

void disk_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->tempint)
			mod = MOD_DISKETTE;
		else
			mod = MOD_DISK;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 20+rand()%40, 400, DAMAGE_ENERGY, mod); // FIXME Mod
	}

	Stick(self, other);

	//G_FreeEdict (self);
}

void fire_disk (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->can_teleport_through_destinations = 1;
	bolt->s.renderfx |= RF_IR_VISIBLE;
	//if (random() < 0.5)
	//{
		//float y = random();

		if (random() < 0.5)
		{ // Disk
			bolt->s.modelindex = ModelIndex ("models/proj/disk/disk.md2");
			bolt->s.modelindex2 = ModelIndex ("models/proj/disk/disk_bottom.md2");

			bolt->s.skinnum = rand()%3;
			bolt->tempint = 0;
		}
		else // Diskette
		{
			bolt->tempint = 1;
			bolt->s.modelindex = ModelIndex ("models/proj/disk/diskette.md2");
		}
	//}
	bolt->owner = self;
	bolt->touch = disk_touch;
	bolt->nextthink = level.time + 12;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


void Weapon_DiskLauncher_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
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

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 0.5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_disk (ent, start, forward, 0, 1000);
	v[YAW]   = ent->client->v_angle[YAW] + 0.5;
	AngleVectors (v, forward, NULL, NULL);
	fire_disk (ent, start, forward, 0, 1000);
	v[YAW]   = ent->client->v_angle[YAW] - 1.5;
	AngleVectors (v, forward, NULL, NULL);
	fire_disk (ent, start, forward, 0, 1000);
	v[YAW]   = ent->client->v_angle[YAW] + 1.5;
	AngleVectors (v, forward, NULL, NULL);
	fire_disk (ent, start, forward, 0, 1000);

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

void Weapon_DiskLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, Weapon_DiskLauncher_Fire);
}
