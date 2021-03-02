#include "g_local.h"
/*
================
Napalm Grenades
================
*/
static void Napalm_Explode (edict_t *ent)
{
	vec3_t		origin;
	//Sean added these 4 vectors	
	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;
	vec3_t   grenade5;
	vec3_t   grenade6;
	vec3_t   grenade7;
	vec3_t   grenade8;
	if (ent->owner->client)	
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,MOD_WF_FLAME);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);	
	if (ent->waterlevel)
	{	
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{	
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// SumFuka did this bit : give grenades up/outwards velocities
	VectorSet(grenade1,20,20,40);
	VectorSet(grenade2,20,-20,40);
	VectorSet(grenade3,-20,20,40);
	VectorSet(grenade4,-20,-20,40);
	VectorSet(grenade5,10,10,40);
	VectorSet(grenade6,10,-10,40);
	VectorSet(grenade7,-10,10,40);
	VectorSet(grenade8,-10,-10,40);
	// Sean : explode the four grenades outwards
/*GREGG
	fire_napalm(ent->owner, origin, grenade1, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade2, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade3, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade4, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade5, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade6, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade7, 120, 10, 8.0, 120);
	fire_napalm(ent->owner, origin, grenade8, 120, 10, 8.0, 120);
*/ //Gregg
	fire_flame(ent->owner, origin, grenade1, 120, 10, 8.0, 120, false);
//	fire_flame(ent->owner, origin, grenade2, 120, 10, 8.0, 120, false);
	fire_flame(ent->owner, origin, grenade3, 120, 10, 8.0, 120, false);
	fire_flame(ent->owner, origin, grenade4, 120, 10, 8.0, 120, false);
//	fire_flame(ent->owner, origin, grenade5, 120, 10, 8.0, 120, false);
	fire_flame(ent->owner, origin, grenade6, 120, 10, 8.0, 120, false);
	fire_flame(ent->owner, origin, grenade7, 120, 10, 8.0, 120, false);
//	fire_flame(ent->owner, origin, grenade8, 120, 10, 8.0, 120, false);


	G_FreeEdict (ent);
}
static void NapalmGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	Napalm_Explode (ent);
}
void fire_napalmgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->wf_team = self->wf_team;
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->s.skinnum = 6;
	grenade->owner = self;
	grenade->touch = NapalmGrenade_Touch; //Stuff for cluster grenades when they explode
	grenade->nextthink = level.time + timer;
	grenade->think = Napalm_Explode; //stuff for cluster grenades exploding
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
			// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = GenericGrenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}
void weapon_napalmgrenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_napalmgrenade (ent, start, forward, damage, 400, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_NapalmGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_napalmgrenadelauncher_fire);
}