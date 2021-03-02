#include "../g_local.h"

void pacman_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int		mod;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (other->takedamage)
	{
		gi.sound (self, CHAN_AUTO, SoundIndex("weapons/pacman/pac_eat_50.wav"), 1, ATTN_NORM, 0);
	}
	//else
	//	gi.sound (self, CHAN_AUTO, SoundIndex("weapons/pacman/pac_bounce.wav"), 1, ATTN_NORM, 0);
}

void Pacman_Eat (edict_t *ent)
{
	edict_t *blip = NULL;

	if (ent->s.frame == 1)
		ent->s.frame = 0;
	else
		ent->s.frame++;

	ent->nextthink = .1;


	while (blip = findradius(blip, ent->s.origin, 100))
	{
		if (blip == ent->owner)
			continue;
		if (!blip->takedamage)
			continue;
		if (blip->solid != SOLID_BBOX)
			continue;
		if (!blip->inuse)
			continue;

		T_Damage (blip, ent, ent->owner, vec3_origin, ent->s.origin, vec3_origin, 8, 0, 0, MOD_PACMAN); // FIXME Mod
		gi.sound (ent, CHAN_AUTO, SoundIndex("weapons/pacman/pac_eat_50.wav"), 1, ATTN_NORM, 0);
	}
}

void fire_pacman (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*bolt;
	trace_t	tr;

	gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/pacman/pac_respawn.wav"), 1, ATTN_NORM, 0);

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_WALLBOUNCE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_TRIGGER;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/proj/pacman/tris.md2");
	bolt->owner = self;
	bolt->touch = pacman_touch;
	bolt->nextthink = level.time + .1;
	bolt->think = Pacman_Eat;
	bolt->nextthink2 = level.time + 20;
	bolt->think2 = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "pacman";
	gi.linkentity (bolt);
	bolt->can_teleport_through_destinations = 1;
	bolt->s.renderfx |= RF_IR_VISIBLE | RF_FULLBRIGHT;
	bolt->s.sound = SoundIndex("weapons/pacman/pac_fly.wav");

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


void Weapon_PacMan_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	fire_pacman (ent, start, forward, 10, 350, 0);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_PacMan (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_PacMan_Fire);
}
