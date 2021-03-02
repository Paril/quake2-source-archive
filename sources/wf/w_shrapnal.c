#include "g_local.h"

/*
=================
Shrapnal Grenade
=================
*/

/*
=================
fire_lead

This is an internal support routine used for Shrapnal based weapons.
fire bullet creates a lot of effects, causing overflows
I copied the beef of the fire_bullet code, but with no effects 
=================
*/
static void fireShrapnal(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);

		//Don't add random spread if it's the sniper riffle
		if (mod != MOD_SNIPERRIFLE)
		{
			VectorMA (end, r, right, end);
			VectorMA (end, u, up, end);
		}

		if (gi.pointcontents (start) & MASK_WATER)
		{
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			VectorCopy (tr.endpos, water_start);
			if (!VectorCompare (start, tr.endpos))
			{				
				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self->owner, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}			
		}
	}
}


static void BulletGrenade_Explode (edict_t *ent)

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
	int		i;
	vec3_t offset;


	if (ent->owner->client)	
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	VectorSet(offset,0,0,32);
	VectorAdd(offset,ent->s.origin,offset);
	VectorCopy (offset, ent->s.origin);
	VectorSet(grenade1,20,20,0);
	VectorSet(grenade2,20,-20,0);
	VectorSet(grenade3,-20,20,0);
	VectorSet(grenade4,-20,-20,0);
	VectorSet(grenade5,0,20,0);
	VectorSet(grenade6,0,-20,0);
	VectorSet(grenade7,-20,0,0);
	VectorSet(grenade8,20,0,0);
	// Sean : explode the four grenades outwards
/*
	fire_bullet (ent->owner, offset, grenade1, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade2, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade3, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade4, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade5, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade6, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade7, 4, 2, 15000, 15000,0);
	fire_bullet (ent->owner, offset, grenade8, 4, 2, 15000, 15000,0);
*/

	// ++TeT - fire bullet creates a lot of effects, causing overflows
	// I copied the beef of the fire_bullet code, but with no effects 
    fire_bullet (ent, offset, grenade1, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade2, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade3, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade4, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade5, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade6, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade7, 4, 2, 15000, 15000,MOD_SHRAPNEL);
    fire_bullet (ent, offset, grenade8, 4, 2, 15000, 15000,MOD_SHRAPNEL);

	// fire the remaining bullets without the effects
	for (i = 0; i < 5; i++) 
	{
        fireShrapnal (ent, offset, grenade1, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade2, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade3, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade4, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade5, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade6, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade7, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
        fireShrapnal (ent, offset, grenade8, wf_game.grenade_damage[GRENADE_TYPE_SHRAPNEL], 2, TE_GUNSHOT, 15000, 15000,MOD_SHRAPNEL);
	}

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	//TeT removed radius damage
	//T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius,MOD_SHRAPNEL);

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
	//T_ShockItems(ent);
	// explode and destroy grenade
	BecomeNewExplosion (ent);	
}

void Shrapnal_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = BulletGrenade_Explode;
}


static void BulletGrenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	ent->nextthink = level.time + .2;
	ent->think = BulletGrenade_Explode;
//	BulletGrenade_Explode (ent);
}

void fire_bulletgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
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
	grenade->s.modelindex = gi.modelindex (GRSHRAPNEL_MODEL);
	grenade->s.skinnum = GRSHRAPNEL_SKIN;

	grenade->owner = self;
	grenade->creator = self;
	grenade->touch = BulletGrenade_Touch; //Stuff for cluster grenades when they explode
	grenade->nextthink = level.time + timer;
	grenade->think = BulletGrenade_Explode; //stuff for cluster grenades exploding
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "shrapnal";
			// CCH: a few more attributes to let the grenade 'die'
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->mass = 2;
	grenade->health = 10;
	grenade->die = Shrapnal_Die;
	grenade->takedamage = DAMAGE_YES;
	grenade->monsterinfo.aiflags = AI_NOSTEP;

	gi.linkentity (grenade);
}
void weapon_shrapnalgrenadelauncher_fire (edict_t *ent)
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

	fire_bulletgrenade (ent, start, forward, damage, 400, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_BulletGrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_shrapnalgrenadelauncher_fire);
}
