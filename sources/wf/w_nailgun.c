#include "g_local.h"
/*
================
Nail Gun
================
*/
void nail_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 2, DAMAGE_ENERGY,MOD_NAIL);
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
void fire_nail (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
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
	bolt->movetype = MOVETYPE_FLYMISSILE; 
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/spike/tris.md2");
	bolt->owner = self;
	bolt->touch = nail_touch;
	bolt->nextthink = level.time + 8;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "nail"; 
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
void weapon_nailgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = wf_game.weapon_damage[WEAPON_NAILGUN];
	int			kick = 8;
	int			speed = wf_game.weapon_speed[WEAPON_NAILGUN];
	if (!ent->client->pers.inventory[ent->client->ammo_index])
		ent->client->ps.gunframe = 12;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	ent->ShotNumber++;
	//fire_nail (ent, start, forward, damage, speed);
	if (ent->client->ping >500)
	{
		if (ent->ShotNumber>1)
		{
			fire_bullet (ent, start, forward, damage*2, 6*2, 25, 25,MOD_NAIL);
			ent->ShotNumber =0;
		}
	}
	else
	{
		fire_bullet (ent, start, forward, damage, 6, 25, 25,MOD_NAIL);
		ent->ShotNumber =0;
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	
	if (ent->client->ps.gunframe == 5 && (ent->client->buttons & BUTTON_ATTACK))
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe++;
	//VWEAP STUFF

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
		ent->client->anim_end = FRAME_attack8;
	}


	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_NailGun (edict_t *ent)
{
	static int	pause_frames[]	= {15, 0};
	static int	fire_frames[]	= {4, 5, 6,7,8,9,10,11,0};

	Weapon_Generic (ent, 3, 11, 14, 18, pause_frames, fire_frames, weapon_nailgun_fire);
}