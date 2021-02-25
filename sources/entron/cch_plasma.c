/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Written by Chris Hilton and Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: cch_plasma.c
  Description: Chris' Plasma Gun

\**********************************************************/

#include "g_local.h"

#define SND_PLASMA_PICKUP     "weapons/plasma/pickup.wav"
#define SND_PLASMA_FIRE       "weapons/plasma/fire.wav"
#define SND_PLASMA_RELOAD     "weapons/plasma/reload.wav"
#define SND_PLASMA_NOAMMO     "weapons/plasma/noammo.wav"
#define SND_PLASMA_WALLHIT    "weapons/plasma/wallhit.wav"

void blob_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_PLASMA);

		G_FreeEdict (self);
	}
	else
	{
        gi.sound(self, CHAN_VOICE, gi.soundindex(SND_PLASMA_WALLHIT), 1, ATTN_NORM, 0);      
	}
//		gi.WriteByte (svc_temp_entity);
//		gi.WriteByte (TE_SCREEN_SPARKS);
//		gi.WritePosition (self->s.origin);
//		if (!plane)
//			gi.WriteDir (vec3_origin);
//		else
//			gi.WriteDir (plane->normal);
//		gi.multicast (self->s.origin, MULTICAST_PVS);
}
/*
void blob_think (edict_t *self)
{
   self->s.frame++;
   if (!self->style--)
      {
      G_FreeEdict(self);
      return;
      }
   if (self->s.frame > 10)
      self->s.frame = 0;
   self->nextthink = level.time + .1;
}
*/

void ThrowPlasmaBlob (edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*blob;
	vec3_t	v;

	blob = G_Spawn();
	VectorCopy (origin, blob->s.origin);
	gi.setmodel (blob, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, blob->velocity);
	blob->owner = self->owner;
	blob->movetype = MOVETYPE_BOUNCE;
	blob->solid = SOLID_BBOX;
	blob->clipmask = MASK_SHOT;
	VectorClear (blob->mins);
	VectorClear (blob->maxs);
	blob->s.effects |= EF_COLOR_SHELL | EF_BFG;
	blob->s.renderfx |= RF_SHELL_GREEN;
	blob->avelocity[0] = random()*600;
	blob->avelocity[1] = random()*600;
	blob->avelocity[2] = random()*600;
	blob->think = G_FreeEdict;
	blob->nextthink = level.time + 1 + random();
	blob->touch = blob_touch;
   blob->style = 10 + rand() & 7;
	blob->dmg = 15;
	blob->s.frame = 0;
	blob->flags = 0;
   blob->mass = 100;
	blob->classname = "plasma blob";
	gi.linkentity (blob);
}

void plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_PLASMA);
	}
	else
	{
		ThrowPlasmaBlob(self, "models/objects/gibs/sm_meat/tris.md2", 2, self->s.origin);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SCREEN_SPARKS);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_COLOR_SHELL | EF_BFG;
	bolt->s.renderfx |= RF_SHELL_GREEN;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex (SND_PLASMA_FIRE);
	bolt->owner = self;
	bolt->touch = plasma_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "plasma bolt";
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

void Plasma_Fire (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_plasma (ent, start, forward, damage, 2000);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_PlasmaGun_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 60;
	else
		damage = 40;
	Plasma_Fire (ent, vec3_origin, damage);
	ent->client->ps.gunframe++;
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_PlasmaGun (edict_t *ent)
{
//	static int	pause_frames[]	= {19, 32, 0};
//	static int	fire_frames[]	= {5, 0};
	static int	pause_frames[]	= {14, 19, 24, 0};
	static int	fire_frames[]	= {6, 0};

//	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_PlasmaGun_Fire);
	Weapon_Generic (ent, 5, 7, 27, 32, pause_frames, fire_frames, Weapon_PlasmaGun_Fire);
}

