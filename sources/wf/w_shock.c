#include "g_local.h"
/*
=================
Shock Grenade
=================
*/
static void shockGrenade_Explode (edict_t *ent)

{
	vec3_t		origin;
//	int		i;
	vec3_t offset;

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

	VectorSet(offset,0,0,0.5);
	VectorAdd(offset,ent->s.origin,offset);
	VectorCopy (offset, ent->s.origin);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	//GREGG - removed radius damage on explode
	//T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_SHOCK);

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

// make some debris
//	make_debris (ent);
	// shake view
//	T_ShockWave(ent, 255, 1024);
	// let blast move items
//	T_ShockItems(ent);
	// explode and destroy grenade
	BecomeNewExplosion (ent);
}

static void shockGrenade_Timer (edict_t *self)
{
	edict_t *ent;
//	vec3_t velo;
	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 340)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if ((ent->wf_team == self->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
			continue;

		//Don't go through walls
		if (!visible(self, ent))
             continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PARASITE_ATTACK);
		gi.WriteShort (self - g_edicts);
		gi.WritePosition (ent->s.origin);
		gi.WritePosition (self->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, wf_game.grenade_damage[GRENADE_TYPE_SHOCK], 0, DAMAGE_BULLET, MOD_SHOCK);
	}
	if(self->volume<level.time)
		self->think =shockGrenade_Explode;
	self->nextthink = level.time + 0.1;
}

static void shockGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	//BulletGrenade_Explode (ent);
}

void shockGrenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = shockGrenade_Explode;
}

void fire_shockgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	++self->client->pers.active_grenades[GRENADE_TYPE_SHOCK];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_SHOCK;
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
	grenade->s.modelindex = gi.modelindex (GRSHOCK_MODEL);
	grenade->s.skinnum = GRSHOCK_SKIN;
	grenade->owner = self;
	grenade->touch = shockGrenade_Touch; //Stuff for cluster grenades when they explode
	grenade->nextthink = level.time + timer;
	grenade->volume = level.time + timer + 0.4;
	grenade->think = shockGrenade_Timer; //stuff for cluster grenades exploding
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
			// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = shockGrenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}
void weapon_shockgrenadelauncher_fire (edict_t *ent)
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

	fire_shockgrenade (ent, start, forward, damage, 400, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_shockGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_shockgrenadelauncher_fire);
}
