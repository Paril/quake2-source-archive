#include "g_local.h"

void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);


// RAFAEL
void fire_blueblaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *bolt;
	trace_t tr;

	VectorNormalize (dir);

	bolt = G_Spawn ();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	bolt->s.modelindex = gi.modelindex ("models/objects/blaser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
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

// RAFAEL

/*
	fire_ionripper
*/

void ionripper_sparks (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (0);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe4 + (rand()&3));
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}

// RAFAEL
void ionripper_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
			PlayerNoise (self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_RIPPER);

	}
	else
	{
		return;
	}

	G_FreeEdict (self);
}


// RAFAEL
void fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *ion;
	trace_t tr;

	VectorNormalize (dir);

	ion = G_Spawn ();
	VectorCopy (start, ion->s.origin);
	VectorCopy (start, ion->s.old_origin);
	vectoangles (dir, ion->s.angles);
	VectorScale (dir, speed, ion->velocity);

//	ion->movetype = MOVETYPE_WALLBOUNCE;
	ion->movetype = MOVETYPE_FLYRICOCHET;
	ion->clipmask = MASK_SHOT;
	ion->solid = SOLID_BBOX;
	ion->s.effects |= effect;

	ion->s.renderfx |= RF_FULLBRIGHT;

	VectorClear (ion->mins);
	VectorClear (ion->maxs);
	ion->s.modelindex = gi.modelindex ("models/objects/boomrang/tris.md2");
	ion->s.sound = gi.soundindex ("misc/lasfly.wav");
	ion->owner = self;
	ion->touch = ionripper_touch;
	ion->nextthink = level.time + 3;
	ion->think = ionripper_sparks;
	ion->dmg = damage;
	ion->dmg_radius = 100;
	gi.linkentity (ion);

	if (self->client)
		check_dodge (self, ion->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, ion->s.origin, ion, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (ion->s.origin, -10, dir, ion->s.origin);
		ion->touch (ion, tr.ent, NULL, NULL);
	}

}


// RAFAEL
/*
fire_heat
*/


void heat_think (edict_t *self)
{
	edict_t		*target = NULL;
	edict_t		*aquire = NULL;
	vec3_t		vec;
	vec3_t		oldang;
	int			len;
	int			oldlen = 0;

	VectorClear (vec);

	// aquire new target
	while (( target = findradius (target, self->s.origin, 1024)) != NULL)
	{
		
		if (self->owner == target)
			continue;
		if (!target->svflags & SVF_MONSTER)
			continue;
		if (!target->client)
			continue;
		if (target->health <= 0)
			continue;
		if (!visible (self, target))
			continue;
		
		// if we need to reduce the tracking cone
		/*
		{
			vec3_t	vec;
			float	dot;
			vec3_t	forward;
	
			AngleVectors (self->s.angles, forward, NULL, NULL);
			VectorSubtract (target->s.origin, self->s.origin, vec);
			VectorNormalize (vec);
			dot = DotProduct (vec, forward);
	
			if (dot > 0.6)
				continue;
		}
		*/

		if (!infront (self, target))
			continue;

		VectorSubtract (self->s.origin, target->s.origin, vec);
		len = VectorLength (vec);

		if (aquire == NULL || len < oldlen)
		{
			aquire = target;
			self->target_ent = aquire;
			oldlen = len;
		}
	}

	if (aquire != NULL)
	{
		VectorCopy (self->s.angles, oldang);
		VectorSubtract (aquire->s.origin, self->s.origin, vec);
		
		vectoangles (vec, self->s.angles);
		
		VectorNormalize (vec);
		VectorCopy (vec, self->movedir);
		VectorScale (vec, 500, self->velocity);
	}

	self->nextthink = level.time + 0.1;
}

// RAFAEL
void fire_heat2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *heat;

	heat = G_Spawn();
	VectorCopy (start, heat->s.origin);
	VectorCopy (dir, heat->movedir);
	vectoangles (dir, heat->s.angles);
	VectorScale (dir, speed, heat->velocity);
	heat->movetype = MOVETYPE_FLYMISSILE;
	heat->clipmask = MASK_SHOT;
	heat->solid = SOLID_BBOX;
	heat->s.effects |= EF_ROCKET;
	VectorClear (heat->mins);
	VectorClear (heat->maxs);
	heat->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	heat->owner = self;
	heat->touch = rocket_touch;

	heat->nextthink = level.time + 0.1;
	heat->think = heat_think;
	
	heat->dmg = damage;
	heat->radius_dmg = radius_damage;
	heat->dmg_radius = damage_radius;
	heat->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (self->client)
		check_dodge (self, heat->s.origin, dir, speed);

	gi.linkentity (heat);
}



// RAFAEL

/*
	fire_plasma
*/

void plasma_touch2 (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_PHALANX);
	}
	
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_PHALANX);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PLASMA_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	G_FreeEdict (ent);
}


// RAFAEL
void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *plasma;

	plasma = G_Spawn();
	VectorCopy (start, plasma->s.origin);
	VectorCopy (dir, plasma->movedir);
	vectoangles (dir, plasma->s.angles);
	VectorScale (dir, speed, plasma->velocity);
	plasma->movetype = MOVETYPE_FLYMISSILE;
	plasma->clipmask = MASK_SHOT;
	plasma->solid = SOLID_BBOX;

	VectorClear (plasma->mins);
	VectorClear (plasma->maxs);
	
	plasma->owner = self;
	plasma->touch = plasma_touch2;
	plasma->nextthink = level.time + 8000/speed;
	plasma->think = G_FreeEdict;
	plasma->dmg = damage;
	plasma->radius_dmg = radius_damage;
	plasma->dmg_radius = damage_radius;
	plasma->s.sound = gi.soundindex ("weapons/rockfly.wav");
	
	plasma->s.modelindex = gi.modelindex ("sprites/s_photon.sp2");
	plasma->s.effects |= EF_PLASMA | EF_ANIM_ALLFAST;
	
	if (self->client)
		check_dodge (self, plasma->s.origin, dir, speed);

	gi.linkentity (plasma);

	
}


// RAFAEL
/*
	RipperGun
*/

void weapon_ionripper_fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right;
	vec3_t	offset;
	vec3_t	tempang;
	int		damage;
	int		kick;

	damage = wf_game.weapon_damage[WEAPON_ION_RIPPER];
	kick = 40;
	
	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	VectorCopy (ent->client->v_angle, tempang);
	tempang[YAW] += crandom();

	AngleVectors (tempang, forward, right, NULL);
	
	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	// VectorSet (offset, 0, 7, ent->viewheight - 8);
	VectorSet (offset, 16, 7, ent->viewheight - 8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_ionripper (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_ION_RIPPER], EF_IONRIPPER);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent - g_edicts);
	gi.WriteByte (MZ_IONRIPPER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise (ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
	
	if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
		ent->client->pers.inventory[ent->client->ammo_index] = 0;
}


void Weapon_Ionripper (edict_t *ent)
{
//	static int pause_frames[] = {36, 0};
//	static int fire_frames[] = {5, 0};

//	Weapon_Generic (ent, 4, 6, 36, 39, pause_frames, fire_frames, weapon_ionripper_fire);

	//Use blaster model for now
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, weapon_ionripper_fire);
}


// 
//	Phalanx
//

void weapon_phalanx_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	vec3_t		v;
	int			kick = 12;
	int			damage;
	float		damage_radius;
	int			radius_damage;

	damage = wf_game.weapon_damage[WEAPON_PHALANX] + (int)(random() * 10.0);
	radius_damage = 120;
	damage_radius = 120;
	
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->ps.gunframe == 8)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] - 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		
		radius_damage = 30;
		damage_radius = 120;
	
		fire_plasma (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_PHALANX], damage_radius, radius_damage);

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] + 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		fire_plasma (ent, start, forward, damage, 725, damage_radius, radius_damage);

		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_PHALANX | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		
		PlayerNoise(ent, start, PNOISE_WEAPON);
	}
	
	ent->client->ps.gunframe++;
	
}

void Weapon_Phalanx (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 55, 0};
	static int	fire_frames[]	= {7, 8, 0};

	Weapon_Generic (ent, 5, 20, 58, 63, pause_frames, fire_frames, weapon_phalanx_fire);

}

