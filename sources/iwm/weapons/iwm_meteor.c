#include "../g_local.h"

void Meteor_Think (edict_t *self)
{
	int big_hairy_ape;
	trace_t	tr;
	vec3_t up = {0, 0, -90};
//	vec3_t vec;
	vec3_t star, end;
//	float dist;
	int i;
	vec3_t check;
	int size = self->wait;
	
	VectorCopy(self->s.origin, star);

	VectorMA(star, 150, up, end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SOLID);

	// Just another ground prevention
	if (tr.fraction == 1.0)
	{
		if (size == 0)
			self->s.origin[2] += 35;
		else
			self->s.origin[2] += 8;
	}

	for(big_hairy_ape = 0; big_hairy_ape < 125; big_hairy_ape++)  // constantly go down
	{
		self->s.origin[2] -= .25;
		M_CheckGround(self);                  // check and see if on ground
		if (self->groundentity)
		{
			self->s.origin[2] += 15;               // if on ground, raise it a bit
			self->groundentity = NULL;
		//	continue;
		}
	}

	VectorCopy (self->s.origin, check);
	if (size == 0)
		check[2] -= 25;
	else if (size == 1)
		check[2] -= 5;
	else
		check[2] -= 5;

	i = gi.pointcontents(check);

	if (i & CONTENTS_SOLID)
	{
		//gi.dprintf ("%d\n", gi.pointcontents(self->s.origin));
		if (size == 0)
			self->s.origin[2] += 10;               // if on ground, raise it a bit
		else if (size == 1)
			self->s.origin[2] += 5;
		else
			self->s.origin[2] += 5;
		self->groundentity = NULL;
	}

	self->nextthink = level.time + .1;
}
void fire_meteor (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int size);

void Meteor_Explode (edict_t *ent)
{
	vec3_t start = {ent->s.origin[0]+(rand()%50-rand()%50), ent->s.origin[1]+(rand()%50-rand()%50), ent->s.origin[2]+(rand()%50-rand()%50)};
	vec3_t start2 = {ent->s.origin[0]+(rand()%50-rand()%50), ent->s.origin[1]+(rand()%50-rand()%50), ent->s.origin[2]+(rand()%50-rand()%50)};
	vec3_t forward = {(rand()%180-rand()%180), (rand()%180+rand()%180), (rand()%180+rand()%180)};
	vec3_t forward2 = {(rand()%180-rand()%180), (rand()%180+rand()%180), (rand()%180+rand()%180)};
	int i;

	// Whoamg HUGE METEOR
	if (ent->wait == 0)
	{
		G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
		T_RadiusDamage (ent, ent->owner, 200, NULL, 200, MOD_BLASTER); // FIXME MOD
		fire_meteor (ent->owner, start, forward, 25, 5, 1);
		G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
		T_RadiusDamage (ent, ent->owner, 200, NULL, 200, MOD_BLASTER); // FIXME MOD
		fire_meteor (ent->owner, start2, forward2, 25, 5, 1);
	}
	else if (ent->wait == 1)
	{
		G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
		T_RadiusDamage (ent, ent->owner, 155, NULL, 155, MOD_BLASTER); // FIXME MOD
		fire_meteor (ent->owner, start, forward, 5, 0, 2);
		G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
		T_RadiusDamage (ent, ent->owner, 155, NULL, 155, MOD_BLASTER); // FIXME MOD
		fire_meteor (ent->owner, start2, forward2, 5, 0, 2);
	}
	else
	{
		for (i = 0; i < 2; i++)
		{
			G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
			T_RadiusDamage (ent, ent->owner, 90, NULL, 90, MOD_BLASTER); // FIXME MOD
		}
	}

	G_FreeEdict(ent);
}

void Meteor_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	ent->enemy = other;
	Meteor_Explode (ent);
}

void fire_meteor (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int size)
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
	VectorCopy (self->s.angles, grenade->s.angles);
	VectorSet (grenade->avelocity, 200, 0, 0);
	grenade->movetype = MOVETYPE_WALLBOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	//VectorClear (grenade->mins);
	//VectorClear (grenade->maxs);
	if (size == 0)
	{
		VectorSet (grenade->mins, -24, -24, -24);
		VectorSet (grenade->maxs, 24, 24, 24);
	}
	else if (size == 1)
	{
		VectorSet (grenade->mins, -16, -16, -16);
		VectorSet (grenade->maxs, 16, 16, 16);
	}
	else
	{
		VectorSet (grenade->mins, -10, -10, -10);
		VectorSet (grenade->maxs, 10, 10, 10);
	}
	grenade->s.modelindex = ModelIndex ("models/proj/meteor/tris.md2");
	grenade->owner = self;
	grenade->touch = Meteor_Touch;
	grenade->nextthink = level.time + .1;
	grenade->think = Meteor_Think;
	grenade->dmg = damage;
	grenade->dmg_radius = 0;
	grenade->classname = "meteor";
	grenade->can_teleport_through_destinations = 1;
	grenade->wait = size;
	grenade->s.frame = size;
	grenade->think2 = Meteor_Explode;
	grenade->nextthink2 = level.time + 3;

	gi.linkentity (grenade);
}

void Weapon_Meteor_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	v[PITCH] = 0;
	AngleVectors (v, forward, NULL, NULL);
	fire_meteor (ent, start, forward, 50, 100, 0);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_meteor (ent, start, forward, 50, 100, 0);

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

void Weapon_Meteor (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, Weapon_Meteor_Fire);
}
