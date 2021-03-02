#include "g_local.h"
/*
================
lightning Gun
================
*/

void lightning_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, self->kick, DAMAGE_BULLET,MOD_LIGHTNING);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (20);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);	
		gi.WriteByte (0xd8dad9db);	//Whiteish yellow looking I think	
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}		
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (20);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);	
		gi.WriteByte (0xd8dad9db);	//Whiteish yellow looking I think		
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int kick)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);
	gi.sound(self, CHAN_VOICE, gi.soundindex("lshoot.wav"), 1, ATTN_NORM, 0);

	bolt = G_Spawn();
	bolt->wf_team = self->wf_team;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE; //JR 1/4/98 changed so it bounces
	bolt->clipmask = MASK_SHOT;
	bolt->s.effects |= EF_TELEPORTER|EF_ANIM01|EF_BLASTER;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/bolt/tris.md2"); //Need to change this
	bolt->s.sound = gi.soundindex ("electric.wav");
	bolt->owner = self;
	bolt->think = G_FreeEdict;
	bolt->nextthink = level.time + 4;
	bolt->touch = lightning_touch;
	bolt->dmg = damage;
	bolt->kick = kick;
	gi.linkentity (bolt);

//	if (self->client)
//		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}

void weapon_lightninggun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	int			speed;

	speed = wf_game.weapon_speed[WEAPON_LIGHTNING]; 
	damage = wf_game.weapon_damage[WEAPON_LIGHTNING];
	kick = 165+ (((int)(random()*1000)) % 30) ;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_lightning (ent,ent->s.origin, forward, damage, speed, kick);
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Lightninggun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_lightninggun_fire);
}