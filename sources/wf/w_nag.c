#include "g_local.h"
/*
==========================
NAG(Nervous Accelator Gun)
==========================
*/
void nag_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float rnum;
	rnum = random();
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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg+1, 2, DAMAGE_ENERGY,MOD_NAG);
		if(rnum < 0.3333)
		{
			ThrowUpNow(other);
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 12, 4, DAMAGE_ENERGY,MOD_NAG);
		}
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

	G_FreeEdict (self);
}

void fire_nag (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->wf_team = self->wf_team;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE; //JR 1/4/98 changed so it bounces
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = nag_touch;
	bolt->nextthink = level.time + 3;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "nag"; 
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

void Nag_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
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

	fire_nag (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_NAG], effect);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	if (hyper)
		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	else
		gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	

}


void Weapon_Nag_Fire (edict_t *ent)
{
	int	damage;
	//JR 1/4/98


	damage = wf_game.weapon_damage[WEAPON_NAG];
	Nag_Fire (ent, vec3_origin, damage, false, EF_BLASTER);

	ent->client->ps.gunframe++;
	
	ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;

}

void Weapon_Nag (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Nag_Fire);
}
