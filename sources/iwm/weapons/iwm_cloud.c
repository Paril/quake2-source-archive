#include "../g_local.h"

void cloud_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_FreeEdict (self);
}


void cloud_think (edict_t *self)
{
	int r;
	int i;

	r = rand()%4;
	i = rand()%4;

	self->s.origin[0] += r -= i;
	self->s.origin[1] += r -= i;

	gi.linkentity (self);

	self->nextthink2 = level.time + 0.3;
}

void acid_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	//if (other == self->owner)
		//return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	//if (self->owner->client)
	//	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		mod = MOD_ACIDDROP;
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY|DAMAGE_NO_ARMOR|DAMAGE_NO_PROTECTION, mod);
	}

	G_FreeEdict (self);
}

void fire_acid (edict_t *self, vec3_t start)
{
	edict_t	*bolt;
//	trace_t	tr;
	vec3_t dir = {0, 0, 0};

	bolt = G_Spawn();
	bolt->svflags = SVF_PROJECTILE;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, 0, bolt->velocity);
	bolt->s.angles[0] = 360;
	bolt->movetype = MOVETYPE_TOSS;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = ModelIndex ("models/other/raindrop/tris.md2");
	bolt->owner = self->owner;
	bolt->touch = acid_touch;
	bolt->classname = "aciddrop";
	bolt->s.renderfx |= RF_FULLBRIGHT;
	bolt->spawnflags = 1;
	bolt->dmg = 8;
	gi.linkentity (bolt);
}	

void cloud_dropacid (edict_t *self)
{
	int r;
	int i;
	vec3_t start = {0, 0, 0};

	r = rand()%50;
	i = rand()%50;

	VectorCopy (self->s.origin, start);

	start[1] += r -= i;
	i -= 10;
	r += 10;
	start[0] += i -= r;

	fire_acid(self, start);

	self->nextthink = level.time + 0.1;
}

void fire_acidcloud (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;
	vec3_t  star;
	vec3_t  st;
	vec3_t  end;
	trace_t tr;
	vec3_t up = {0, 0, 90};
	
	VectorCopy(self->s.origin, star);

	VectorMA(star, 8192, up, end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SOLID);

	if ( gi.pointcontents(tr.endpos) == (CONTENTS_SOLID || CONTENTS_WINDOW || CONTENTS_TRANSLUCENT || CONTENTS_LADDER))
	{
		gi.dprintf ("Somehow, the cloud went inside a wall. Tell Paril to fix it damnit.\n");
		return;
	}

	VectorCopy (tr.endpos, st);

	//st[2] -= 50;

	bfg = G_Spawn();
	VectorCopy (st, bfg->s.origin);
	bfg->movetype = MOVETYPE_NONE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = ModelIndex ("models/other/cloud/tris.md2");
	bfg->owner = self;
	bfg->touch = cloud_touch;
	bfg->classname = "cloud";
	bfg->s.effects |= EF_SPHERETRANS;
	//bfg->s.renderfx |= RF_TRANSLUCENT;

	bfg->think = cloud_dropacid;
	bfg->nextthink = level.time + 0.7;
	bfg->think2 = cloud_think;
	bfg->nextthink2 = level.time + 0.7;
	bfg->think3 = G_FreeEdict;
	bfg->nextthink3 = level.time + 20;
	bfg->s.renderfx |= RF_IR_VISIBLE;

	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	gi.linkentity (bfg);
}


void cloud_findenemyandZAP (edict_t *e)
{
	edict_t *blip = NULL;
	trace_t tr;

	while (blip = findradius(blip, e->s.origin, 800))
	{
		// Don't target itself
		if (blip == e)
			continue;
		// Don't target owner
		if (blip == e->owner)
			continue;
		// Don't target item (not needed?)
		if (blip->item)
			continue;
		// Don't target non-solids
		if (blip->solid != SOLID_BBOX)
			continue;
		// If there is no blip..
		if (!blip->inuse)
			continue;;
		// No targetting clouds
		if (Q_stricmp(blip->classname, "lightcloud") == 0 ||
			Q_stricmp(blip->classname, "acidcloud") == 0 ||
			Q_stricmp(blip->classname, "info_player_start") == 0 ||
			Q_stricmp(blip->classname, "info_player_deathmatch") == 0 ||
			Q_stricmp(blip->classname, "info_player_coop") == 0 ||
			Q_stricmp(blip->classname, "noclass") == 0)
			continue;
		if (!visible(e, blip))
			continue;
		// Don't target owner's car
		if (Q_stricmp(blip->classname, "monster_car") == 0 && (e->owner->vehicle == blip))
			continue;
		if (!blip->takedamage)
			continue;

		// Woot, we found a blip
		//gi.dprintf ("%s\n", blip->classname);

		T_Damage(blip, e, e->owner, vec3_origin, blip->s.origin, vec3_origin, 25, 5, 0, MOD_ZAPPED); // FIXME Mod

		tr = gi.trace (e->s.origin, NULL, NULL, blip->s.origin, e, MASK_SHOT);
		if (tr.ent)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_LIGHTNING);
			gi.WriteShort (blip - g_edicts);			// destination entity
			gi.WriteShort (e - g_edicts);		// source entity
			gi.WritePosition (tr.endpos);
			gi.WritePosition (e->s.origin);
			gi.multicast (e->s.origin, MULTICAST_PVS);
		}

		//VectorCopy (tr.endpos, start);
	}
	e->nextthink = level.time + 1;
}

void fire_lightningcloud (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;
	vec3_t  star;
	vec3_t  st;
	vec3_t  end;
	trace_t tr;
	vec3_t up = {0, 0, 90};
	
	VectorCopy(self->s.origin, star);

	VectorMA(star, 8192, up, end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SOLID);

	if ( gi.pointcontents(tr.endpos) == (CONTENTS_SOLID || CONTENTS_WINDOW || CONTENTS_TRANSLUCENT || CONTENTS_LADDER))
	{
		gi.dprintf ("Somehow, the cloud went inside a wall. Tell Paril to fix it damnit.\n");
		return;
	}

	VectorCopy (tr.endpos, st);

	//st[2] -= 50;

	bfg = G_Spawn();
	VectorCopy (st, bfg->s.origin);
	bfg->movetype = MOVETYPE_NONE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = ModelIndex ("models/other/cloud/tris.md2");
	bfg->owner = self;
	bfg->touch = cloud_touch;
	bfg->classname = "lightcloud";
	bfg->s.effects |= EF_SPHERETRANS;
	//bfg->s.renderfx |= RF_TRANSLUCENT;

	bfg->think = cloud_findenemyandZAP;
	bfg->nextthink = level.time + 0.1;
	bfg->think2 = cloud_think;
	bfg->nextthink2 = level.time + 0.7;
	bfg->think3 = G_FreeEdict;
	bfg->nextthink3 = level.time + 20;

	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	gi.linkentity (bfg);
}


void Weapon_AcidCloud_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	fire_acidcloud (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_AcidCloud (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, Weapon_AcidCloud_Fire);
}


void Weapon_LightningCloud_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	fire_lightningcloud (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_LightningCloud (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, Weapon_LightningCloud_Fire);
}
