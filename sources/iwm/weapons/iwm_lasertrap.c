#include "g_local.h"
#include "m_player.h"

/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) ;

void lasertraplaser_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}

	VectorInverse(self->velocity);
	gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/lozarhit.wav"), 1, ATTN_IDLE, 0);
	G_Spawn_Splash (TE_TUNNEL_SPARKS, 45, COLOR_RED2, self->s.origin, self->velocity, self->s.origin);

	G_FreeEdict (self);
}

void LazerTrapLaser_Think (edict_t *self)
{
	vec3_t forward;
	edict_t *ent = self;

	if (self->tempvec[0] != 9999)
	{
		VectorNormalize(self->tempvec);
		VectorScale(self->tempvec, 15, self->tempvec);
		VectorAdd(self->tempvec, ent->movedir, self->tempvec);
		VectorNormalize(self->tempvec);
		VectorCopy(self->tempvec, ent->movedir);
		vectoangles(self->tempvec, ent->s.angles);
//		speed = VectorLength(ent->velocity);
		VectorScale(self->tempvec, 2000, ent->velocity);		
	}
	else
	{
		AngleVectors (self->s.angles, NULL, NULL, forward);
	//VectorMA (self->velocity, -100, forward, self->velocity);
		self->velocity[2] -= 600;
	}

	self->nextthinks[1] = level.time + .1;
}

void LaserTrap_FindQuickPlayerToHome (edict_t *ent)
{
	edict_t *blip = NULL;
//	vec3_t mid;
//	vec3_t line;
//	vec3_t angles;

	VectorSet (ent->tempvec, 9999, 9999, 9999);

	while (blip = findradius(blip, ent->s.origin, 1000))
	{
		if (blip->takedamage && random() > 0.6 && blip != ent->owner)
		{
		//	VectorCopy (blip->s.origin, ent->tempvec);
		 VectorSubtract(blip->s.origin, ent->s.origin, ent->tempvec);
		}
	}
}

void LazerTrapLaser_QuickThink (edict_t *self)
{
	vec3_t launchdir = {rand()%15-rand()%15, rand()%15-rand()%15, self->velocity[2]};
	VectorCopy (launchdir, self->velocity);
	self->velocity[2] = 0;
	LaserTrap_FindQuickPlayerToHome(self);
	self->thinks[1] = LazerTrapLaser_Think;
	self->nextthinks[1] = level.time + .1;
}

void LaserTrapHit (edict_t *ent)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, ent->s.origin, 125))
	{
		if (blip->takedamage)
		{
			lasertraplaser_touch(ent, blip, NULL, NULL);
			return;
		}
	}
	ent->nextthinks[3] = level.time + .1;
}

void LaserTrapFindTargetToExplodeOnLol (edict_t *ent)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, ent->s.origin, 235))
	{
		if (blip->takedamage && blip != ent->owner)
		{
			ent->nextthinks[0] = level.time + .1;
			return;
		}
	}
	ent->nextthinks[3] = level.time + .1;

}
void fire_lasertraplaser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hy)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	if (random() < 0.4)
	gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/lazor.wav"), 1, ATTN_STATIC, 0);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_PENT;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/objects/lazor/tris.md2");
	bolt->owner = self;
	bolt->touch = lasertraplaser_touch;
	bolt->thinks[1] = LazerTrapLaser_QuickThink;
	bolt->nextthinks[1] = level.time + .6;
	bolt->thinks[3] = LaserTrapHit;
	bolt->nextthinks[3] = level.time + .1;
	bolt->dmg = damage;
	bolt->classname = "lazor";
	if (hy)
		bolt->spawnflags = 1;
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);
	bolt->can_teleport_through_destinations = 1;

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}
void fire_lasertrap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);

void lasertrap_Explode (edict_t *ent)
{
//	vec3_t		origin;
//	int			mod;
	int lazors = 0;

	if (ent->dmg == 666)
	{
		ent->s.origin[2] += 14;
		fire_lasertrap (ent, ent->s.origin, vec3_origin, 444, 0, 35, 0, false);
		BecomeExplosion1(ent);
		return;
	}

	//ent->s.origin[2] += 4;

	for (; lazors < 24+rand()%12; lazors++)
	{
		vec3_t launchdir = {rand()%40-rand()%40, rand()%40-rand()%40, 90};
		fire_lasertraplaser (ent->owner, ent->s.origin, launchdir, 25, 1000, 0, false);
	}

	G_FreeEdict (ent);
}

void lasertrap_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
	lasertrap_Explode (ent);
}

void fire_lasertrap (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
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
	//VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = ModelIndex (damage == 444 ? "models/objects/lasertrap/tris.md2" : "models/objects/grenade2/tris.md2");
	grenade->owner = damage == 444 ? self->owner : self;
	grenade->touch = lasertrap_Touch;
	grenade->nextthinks[0] = level.time + timer;
	grenade->thinks[0]= lasertrap_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	if (damage == 444)
	{
	//	grenade->s.effects = EF_SPHERETRANS;
		//grenade->s.renderfx = RF_TRANSLUCENT;
		grenade->thinks[3] = LaserTrapFindTargetToExplodeOnLol;
		grenade->nextthinks[3] = level.time + .1;
	}
	else
	grenade->s.sound = SoundIndex("weapons/hgrenc1b.wav");

	grenade->can_teleport_through_destinations = 1;

	if (timer <= 0.0)
		lasertrap_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, SoundIndex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}

	if (damage == 444)
		VectorClear (grenade->velocity);
}

void weapon_lasertrap_fire (edict_t *ent, qboolean held)
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

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	if (ent->client->pers.weapon == FindItem("Tesla"))
	{
//		VectorSet(offset, 0, -12, ent->viewheight-26);
		VectorSet(offset, 0, -4, ent->viewheight-22);
	}
	else
	{
//		VectorSet(offset, 8, 8, ent->viewheight-8);
		VectorSet(offset, 2, 6, ent->viewheight-14);
	}
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_lasertrap (ent, start, forward, 666, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || (ent->s.modelindex != 255 && !ent->client->cloak_turninvis_framenum)) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_LaserTrap (edict_t *ent)
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
			gi.sound(ent, CHAN_WEAPON, SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = SoundIndex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_lasertrap_fire (ent, true);
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
			weapon_lasertrap_fire (ent, false);
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