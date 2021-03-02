#include "g_local.h"
/*
=================
Slow Grenade
=================
*/
static void SlowGrenade_Explode (edict_t *ent)

{
	vec3_t		origin;
//	int		i;
	vec3_t offset;


	if (ent->owner->client)	
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	VectorSet(offset,0,0,0.5);
	VectorAdd(offset,ent->s.origin,offset);
	VectorCopy (offset, ent->s.origin);
	
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,0);

	VectorMA (ent->s.origin, -.02, ent->velocity, origin);
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
	
// make some debris
	//make_debris (ent);
	// shake view
	T_ShockWave(ent, 255, 1024);
	// let blast move items
//	T_ShockItems(ent);
	// explode and destroy grenade
	BecomeNewExplosion (ent);	
}

static void SlowGrenade_Timer (edict_t *self)
{
	edict_t *ent;
	vec3_t velo;
	float Distance;
	float Amount;
	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 1024)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;
		VectorSubtract(ent->s.origin,self->s.origin, velo);
		Distance = VectorLength(velo);
		Amount = 20 *(Distance/256);
		if(Amount <1)
			Amount = 1;
		ent->Slower += Amount;
	}
	if(self->volume<level.time)
		self->think =SlowGrenade_Explode;
	self->nextthink = level.time + 0.1;
}


static void SlowGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

}


void fire_slowgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade3/tris.md2");
	grenade->s.skinnum = 6;

	grenade->owner = self;
	grenade->touch = SlowGrenade_Touch; //Stuff for cluster grenades when they explode
	grenade->nextthink = level.time + timer;
	grenade->volume = level.time + timer + 3.5;
	grenade->think = SlowGrenade_Timer; //stuff for cluster grenades exploding
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
void weapon_slowgrenade_fire (edict_t *ent)
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

	fire_slowgrenade (ent, start, forward, damage, 400, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_SlowGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_slowgrenade_fire);
}