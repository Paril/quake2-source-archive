#include "g_local.h"
/*
===============
Plague Grenades
===============
*/

void DiseaseGrenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t *target;
//	int rnd;

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
//		--ent->owner->client->pers.active_grenades[GRENADE_TYPE_PLAGUE];

	}
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
//	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,0);

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
	target= NULL;
	
	while ((target = findradius(target, ent->s.origin, 95)) != NULL)
	{
		//Don't go through walls
		if (!visible(ent, target))
             continue;

		//Give them only a 1/3 chance of being infected
		//rnd = rndnum(1,3);
		//if (rnd == 1)
		infect_person(target, ent->owner);	
	}

	//Just infect, don't damage
//	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,0);

// make some debris
//	make_debris (ent);
	// shake view
	T_ShockWave(ent, 255, 1024);
	// let blast move items
//	T_ShockItems(ent);
	// explode and destroy grenade
	//Also spray out stuff for looks
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
   	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/rocklx1a.wav"), 1, ATTN_NORM, 0);

	// Blow up the grenade
   //BecomeExplosion1(ent);
	G_FreeEdict (ent);
	return;	

}
void DiseaseGrenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = DiseaseGrenade_Explode;
}


void DiseaseGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
//		if (ent->owner->client)
//			--ent->owner->client->pers.active_grenades[GRENADE_TYPE_PLAGUE];
		G_FreeEdict (ent);
		return;
	}

//	if (other->takedamage)
//	{
//		ent->enemy = other;
		DiseaseGrenade_Explode(ent);
//	}
}



void fire_diseasegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

//	++self->client->pers.active_grenades[GRENADE_TYPE_PLAGUE];

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
//	grenade->s.effects |= EF_GRENADE;
	grenade->s.effects |= EF_DOUBLE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
//	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->s.modelindex = gi.modelindex (GRPLAGUE_MODEL);
	grenade->s.skinnum = GRPLAGUE_SKIN;
	grenade->owner = self;
	grenade->touch = DiseaseGrenade_Touch; 
	grenade->nextthink = level.time + 4;
	grenade->think = DiseaseGrenade_Explode; 
	grenade->dmg = (damage/3);
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = DiseaseGrenade_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;
	grenade->wf_team = self->wf_team;

	gi.linkentity (grenade);
}

void weapon_diseaselauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 20;
	float	radius;

	radius = damage+5;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_diseasegrenade (ent, start, forward, damage, 300, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_DiseaseGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_diseaselauncher_fire);
}