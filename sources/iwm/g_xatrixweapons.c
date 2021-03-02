#include "g_local.h"
#include "m_player.h"

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

	ion->movetype = MOVETYPE_WALLBOUNCE;
	ion->clipmask = MASK_SHOT;
	ion->solid = SOLID_BBOX;
	ion->s.effects |= effect;

	ion->s.renderfx |= RF_FULLBRIGHT;

	VectorClear (ion->mins);
	VectorClear (ion->maxs);
	ion->s.modelindex = ModelIndex ("models/objects/boomrang/tris.md2");
	ion->s.sound = SoundIndex ("misc/lasfly.wav");
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
	fire_plasma
*/

void phalanx_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
void fire_phalanx (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	plasma->touch = phalanx_touch;
	plasma->nextthink = level.time + 8000/speed;
	plasma->think = G_FreeEdict;
	plasma->dmg = damage;
	plasma->radius_dmg = radius_damage;
	plasma->dmg_radius = damage_radius;
	plasma->s.sound = SoundIndex ("weapons/rockfly.wav");
	
	plasma->s.modelindex = ModelIndex ("sprites/s_photon.sp2");
//	gi.dprintf ("Called it!\n");
	plasma->s.effects |= EF_PLASMA | EF_ANIM_ALLFAST;
	
	if (self->client)
		check_dodge (self, plasma->s.origin, dir, speed);

	gi.linkentity (plasma);

	
}

// RAFAEL

// RAFAEL
void SP_item_foodcube (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/objects/trapfx/tris.md2";
	SpawnItem (self, FindItem ("Health"));
	self->spawnflags |= DROPPED_ITEM;
	self->style = 1;
	SoundIndex ("items/s_health.wav");
	self->classname = "foodcube";
}

// RAFAEL
void Trap_Think (edict_t *ent)
{
	edict_t	*target = NULL;
	edict_t	*best = NULL;
	vec3_t	vec;
	int		len, i;
	int		oldlen = 8000;
	vec3_t	forward, right, up;
	
	if (ent->timestamp < level.time)
	{
		BecomeExplosion1(ent);
		// note to self
		// cause explosion damage???
		return;
	}
	
	ent->nextthink = level.time + 0.1;
	
	if (!ent->groundentity)
		return;

	// ok lets do the blood effect
	if (ent->s.frame > 4)
	{
		if (ent->s.frame == 5)
		{
			if (ent->wait == 64)
				gi.sound(ent, CHAN_VOICE, SoundIndex ("weapons/trapdown.wav"), 1, ATTN_IDLE, 0);

			ent->wait -= 2;
			ent->delay += level.time;

			for (i=0; i<3; i++)
			{
				
				best = G_Spawn();

				if (strcmp (ent->enemy->classname, "monster_gekk") == 0)
				{
					best->s.modelindex = ModelIndex ("models/objects/gekkgib/torso/tris.md2");	
					best->s.effects |= TE_GREENBLOOD;
				}
				else if (ent->mass > 200)
				{
					best->s.modelindex = ModelIndex ("models/objects/gibs/chest/tris.md2");	
					best->s.effects |= TE_BLOOD;
				}
				else
				{
					best->s.modelindex = ModelIndex ("models/objects/gibs/sm_meat/tris.md2");	
					best->s.effects |= TE_BLOOD;
				}

				AngleVectors (ent->s.angles, forward, right, up);
				
				RotatePointAroundVector( vec, up, right, ((360.0/3)* i)+ent->delay);
				VectorMA (vec, ent->wait/2, vec, vec);
				VectorAdd(vec, ent->s.origin, vec);
				VectorAdd(vec, forward, best->s.origin);
  
				best->s.origin[2] = ent->s.origin[2] + ent->wait;
				
				VectorCopy (ent->s.angles, best->s.angles);
  
				best->solid = SOLID_NOT;
				best->s.effects |= EF_GIB;
				best->takedamage = DAMAGE_YES;
		
				best->movetype = MOVETYPE_TOSS;
				best->svflags |= SVF_MONSTER;
				best->deadflag = DEAD_DEAD;
		  
				VectorClear (best->mins);
				VectorClear (best->maxs);

				best->watertype = gi.pointcontents(best->s.origin);
				if (best->watertype & MASK_WATER)
					best->waterlevel = 1;

				best->nextthink = level.time + 0.1;
				best->think = G_FreeEdict;
				gi.linkentity (best);
			}
				
			if (ent->wait < 19)
				ent->s.frame ++;

			return;
		}
		ent->s.frame ++;
		if (ent->s.frame == 8)
		{
			ent->nextthink = level.time + 1.0;
			ent->think = G_FreeEdict;

			best = G_Spawn ();
			SP_item_foodcube (best);
			VectorCopy (ent->s.origin, best->s.origin);
			best->s.origin[2]+= 16;
			best->velocity[2] = 400;
			best->count = ent->mass;
			gi.linkentity (best);
			return;
		}
		return;
	}
	
	ent->s.effects &= ~EF_TRAP;
	if (ent->s.frame >= 4)
	{
		ent->s.effects |= EF_TRAP;
		VectorClear (ent->mins);
		VectorClear (ent->maxs);

	}

	if (ent->s.frame < 4)
		ent->s.frame++;

	while ((target = findradius(target, ent->s.origin, 256)) != NULL)
	{
		if (target == ent)
			continue;
		if (!(target->svflags & SVF_MONSTER) && !target->client)
			continue;
		// if (target == ent->owner)
		//	continue;
		if (target->health <= 0)
			continue;
		if (!visible (ent, target))
		 	continue;
		if (!best)
		{
			best = target;
			continue;
		}
		VectorSubtract (ent->s.origin, target->s.origin, vec);
		len = VectorLength (vec);
		if (len < oldlen)
		{
			oldlen = len;
			best = target;
		}
	}

	// pull the enemy in
	if (best)
	{
		vec3_t	forward;

		if (best->groundentity)
		{
			best->s.origin[2] += 1;
			best->groundentity = NULL;
		}
		VectorSubtract (ent->s.origin, best->s.origin, vec);
		len = VectorLength (vec);
		if (best->client)
		{
			VectorNormalize (vec);
			VectorMA (best->velocity, 250, vec, best->velocity);
		}
		else
		{
			best->ideal_yaw = vectoyaw(vec);	
			M_ChangeYaw (best);
			AngleVectors (best->s.angles, forward, NULL, NULL);
			VectorScale (forward, 256, best->velocity);
		}

		gi.sound(ent, CHAN_VOICE, SoundIndex ("weapons/trapsuck.wav"), 1, ATTN_IDLE, 0);
		
		if (len < 32)
		{	
			if (best->mass < 400)
			{
				T_Damage (best, ent, ent->owner, vec3_origin, best->s.origin, vec3_origin, 100000, 1, 0, MOD_TRAP);
				ent->enemy = best;
				ent->wait = 64;
				VectorCopy (ent->s.origin, ent->s.old_origin);
				ent->timestamp = level.time + 30;
				if (deathmatch->value)
					ent->mass = best->mass/4;
				else
					ent->mass = best->mass/10;
				// ok spawn the food cube
				ent->s.frame = 5;	
			}
			else
			{
				BecomeExplosion1(ent);
				// note to self
				// cause explosion damage???
				return;
			}
				
		}
	}

		
}


// RAFAEL
void fire_trap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*trap;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	trap = G_Spawn();
	VectorCopy (start, trap->s.origin);
	VectorScale (aimdir, speed, trap->velocity);
	VectorMA (trap->velocity, 200 + crandom() * 10.0, up, trap->velocity);
	VectorMA (trap->velocity, crandom() * 10.0, right, trap->velocity);
	VectorSet (trap->avelocity, 0, 300, 0);
	trap->movetype = MOVETYPE_BOUNCE;
	trap->clipmask = MASK_SHOT;
	trap->solid = SOLID_BBOX;
//	VectorClear (trap->mins);
//	VectorClear (trap->maxs);
	VectorSet (trap->mins, -4, -4, 0);
	VectorSet (trap->maxs, 4, 4, 8);
	trap->s.modelindex = ModelIndex ("models/weapons/z_trap/tris.md2");
	trap->owner = self;
	trap->nextthink = level.time + 1.0;
	trap->think = Trap_Think;
	trap->dmg = damage;
	trap->dmg_radius = damage_radius;
	trap->classname = "trap";
	// RAFAEL 16-APR-98
	trap->s.sound = SoundIndex ("weapons/traploop.wav");
	// END 16-APR-98
	if (held)
		trap->spawnflags = 3;
	else
		trap->spawnflags = 1;
	
	if (timer <= 0.0)
		Grenade_Explode (trap);
	else
	{
		// gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/trapdown.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (trap);
	}
	
	trap->timestamp = level.time + 30;

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

	if (deathmatch->value)
	{
		// tone down for deathmatch
		damage = 30;
		kick = 40;
	}
	else
	{
		damage = 50;
		kick = 60;
	}
	
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

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	fire_ionripper (ent, start, forward, damage, 500, EF_IONRIPPER);

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
	static int pause_frames[] = {36, 0};
	static int fire_frames[] = {5, 0};

	Weapon_Generic (ent, 4, 6, 36, 39, pause_frames, fire_frames, weapon_ionripper_fire);
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
	int			damage;
	float		damage_radius;
	int			radius_damage;

	damage = 70 + (int)(random() * 10.0);
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (ent->client->ps.gunframe == 8)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] - 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		
		radius_damage = 30;
		damage_radius = 120;
	
		fire_phalanx (ent, start, forward, damage, 725, damage_radius, radius_damage);

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] + 1.5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, right, up);
		fire_phalanx (ent, start, forward, damage, 725, damage_radius, radius_damage);

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

/*
======================================================================

TRAP

======================================================================
*/

#define TRAP_TIMER			5.0
#define TRAP_MINSPEED		300
#define TRAP_MAXSPEED		700

void weapon_trap_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	// fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);
	fire_trap (ent, start, forward, damage, speed, timer, radius, held);
	
// you don't get infinite traps!  ZOID
//	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )

	ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_Trap (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			// RAFAEL 16-APR-98
			// gi.sound(ent, CHAN_WEAPON, SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
			gi.sound(ent, CHAN_WEAPON, SoundIndex("weapons/trapcock.wav"), 1, ATTN_NORM, 0);
			// END 16-APR-98

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				// RAFAEL 16-APR-98
				ent->client->weapon_sound = SoundIndex("weapons/traploop.wav");
				// END 16-APR-98
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_trap_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_trap_fire (ent, false);
			if (ent->client->pers.inventory[ent->client->ammo_index] == 0)
				NoAmmoWeaponChange (ent);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}
