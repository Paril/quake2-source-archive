/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Written by Chris Hilton and Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: cch_ripper.c
  Description: Chris' Ripper

\**********************************************************/

#include "g_local.h"

#define SND_RIPPER_PICKUP     "weapons/ripper/pickup.wav"
#define SND_RIPPER_SPIN       "weapons/ripper/engine.wav"
#define SND_RIPPER_ENGINE     "weapons/ripper/engine.wav"
#define SND_RIPPER_FIRE       "weapons/ripper/fire.wav"
#define SND_RIPPER_SPINDOWN   "weapons/ripper/spindowm.wav"
#define SND_RIPPER_SPINUP     "weapons/ripper/spinup.wav"
#define SND_RIPPER_RELOAD     "weapons/ripper/reload.wav"
#define SND_RIPPER_NOAMMO     "weapons/ripper/noammo.wav"
#define SND_RIPPER_SHATTER    "weapons/ripper/shatter.wav"

void shard_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->oldenemy->client)
		PlayerNoise(self->oldenemy, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->oldenemy, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_RIPPER);

		G_FreeEdict (self);
	}
}

void ThrowDiscShard (edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*shard;
	vec3_t	v;

	shard = G_Spawn();
	VectorCopy (origin, shard->s.origin);
	gi.setmodel (shard, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, shard->velocity);
	shard->owner = self->owner;				// this isn't the real owner, so we
	shard->oldenemy = self->oldenemy;		// copy trick to hide real owner from the ripper disc
	shard->movetype = MOVETYPE_BOUNCE;
	shard->solid = SOLID_BBOX;
	shard->clipmask = MASK_SHOT;
	VectorClear (shard->mins);
	VectorClear (shard->maxs);
	shard->avelocity[0] = random()*600;
	shard->avelocity[1] = random()*600;
	shard->avelocity[2] = random()*600;
	shard->think = G_FreeEdict;
	shard->nextthink = level.time + 1 + random()*1;
	shard->touch = shard_touch;
	shard->dmg = 10;
	gi.linkentity (shard);
}

void ripper_die (edict_t *self)
{
	// throw some debris
	ThrowDebris(self, "models/objects/debris2/tris.md2", 2, self->s.origin);

	// make a breaking sound
	gi.sound(self, CHAN_AUTO, gi.soundindex (SND_RIPPER_SHATTER), 1, ATTN_NORM, 0);

	G_FreeEdict(self);
}

void ripper_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (self != self->owner && other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	/*
	if (surf && (surf->flags & TEXTURE_DIRT))
		{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 3;
		self->movetype = MOVETYPE_NONE;
		self->touch = NULL;
		//self->velocity[0] = 0;
		//self->velocity[1] = 0;
		//self->velocity[2] = 0;
		return;
		}
*/
	if ( self->owner != self )
	{
		self->oldenemy = self->owner;	// trick to hide owner from world
		self->owner = self;
	}

	if (self->oldenemy->client)
		PlayerNoise(self->oldenemy, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		// trick to make other temporary owner so disc will keep flying
		self->owner = other;
		T_Damage (other, self, self->oldenemy, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_RIPPER);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	return;
}

void fire_ripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*disc;
	trace_t	tr;

	VectorNormalize (dir);

	disc = G_Spawn();
	VectorCopy (start, disc->s.origin);
	VectorCopy (start, disc->s.old_origin);
	vectoangles (dir, disc->s.angles);
	if (self->client->ps.gunframe == 14)
      disc->s.angles[2] += 90;
	VectorScale (dir, speed, disc->velocity);
	disc->movetype = MOVETYPE_FLYRICOCHET;
	disc->clipmask = MASK_SHOT;
	disc->solid = SOLID_BBOX;
	VectorSet(disc->mins, -7, -7, -1);
	VectorSet(disc->maxs, 7, 7, 1);
	disc->s.modelindex = gi.modelindex ("models/weapons/ripperdisc/tris.md2");
	disc->s.sound = gi.soundindex (SND_RIPPER_FIRE);
	disc->owner = self;
	disc->touch = ripper_touch;
	disc->nextthink = level.time + 1;
	disc->think = ripper_die;
	disc->dmg = damage;
	gi.linkentity (disc);

	if (self->client)
		check_dodge (self, disc->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, disc->s.origin, disc, MASK_SHOT);
	if (tr.fraction < 1.0)
	   {
		VectorMA (disc->s.origin, -10, dir, disc->s.origin);
		disc->touch (disc, tr.ent, NULL, NULL);
	   }
}	

void Weapon_Ripper_Fire (edict_t *ent)
{
	vec3_t	offset, forward, right, start;
	int		damage;

	ent->client->weapon_sound = gi.soundindex(SND_RIPPER_FIRE);

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if ( ent->client->ps.gunframe >= 4 )
		{
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;

			if (is_quad)
				damage *= 4;
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorSet(offset, 24, 8, ent->viewheight-8);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;

			fire_ripper (ent, start, forward, damage, 1000);

			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			PlayerNoise(ent, start, PNOISE_WEAPON);

			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;
		}
		ent->client->ps.gunframe = 9;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_RIPPER_NOAMMO), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 9 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 4;
	}

	if (ent->client->ps.gunframe == 9)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex(SND_RIPPER_FIRE), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_Ripper_Fire2 (edict_t *ent)
{
	vec3_t	offset, forward, right, start;
	int		damage;

	ent->client->weapon_sound = gi.soundindex(SND_RIPPER_ENGINE);

	//ent->s.frame--;
	if (!(ent->client->buttons & BUTTON_ATTACK) )
		{
		ent->client->ps.gunframe++;
		}
	else
		{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
			{
			if (level.time >= ent->pain_debounce_time)
				{
				gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_RIPPER_NOAMMO), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
				}
			NoAmmoWeaponChange (ent);
			}
		else if ( (ent->client->ps.gunframe + 1) % 5 != 0 )
			{
			ent->client->ps.gunframe++;
			if ( ent->client->ps.gunframe == 9 )
				ent->client->ps.gunframe = 4;
			}
		else
			{
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;

			if (is_quad)
				damage *= 4;
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorSet(offset, 24, 8, ent->viewheight-8);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;

			fire_ripper (ent, start, forward, damage, 1000);

			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			PlayerNoise(ent, start, PNOISE_WEAPON);

			ent->client->ps.gunframe++;
			if (ent->client->ps.gunframe == 9 && ent->client->pers.inventory[ent->client->ammo_index])
				ent->client->ps.gunframe = 4;

			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;
			}
		}

	if (ent->client->ps.gunframe == 20)
		{
		gi.sound(ent, CHAN_AUTO, gi.soundindex(SND_RIPPER_FIRE), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
		}

}

void Weapon_Ripper (edict_t *ent)
{
	static int	pause_frames[]	= {26, 33, 41, 0};
	static int	fire_frames[]	= {9, 14, 19, 0};

	Weapon_Generic (ent, 5, 20, 45, 51, pause_frames, fire_frames, Weapon_Ripper_Fire2);
}


