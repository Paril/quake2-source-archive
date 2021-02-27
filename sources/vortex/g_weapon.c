#include "g_local.h"

void bfg_think (edict_t *self);//K03
/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
//K03 Begin
extern qboolean	is_quad;

qboolean G_EntValid(edict_t *ent)
{
	// entity must exist and be in-use
	if (!ent || !ent->inuse)
		return false;
	// if this is not a player-monster (morph), then the entity must be solid/damageable
	if (!PM_PlayerHasMonster(ent) && (!ent->takedamage || (ent->solid == SOLID_NOT)))
		return false;
	return true;
}
qboolean visible (edict_t *self, edict_t *other)
{
	vec3_t start, end;

	G_EntViewPoint(self, start);
	G_EntMidPoint(other, end);

	return ((gi.inPVS(start, end)) && (gi.trace(start, vec3_origin, vec3_origin, end, self, MASK_SOLID).fraction == 1.0));
}

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed, int radius)
{
	vec3_t	end;
	vec3_t	v;
	vec3_t	zvec = {0,0,0};
	trace_t	tr;
	float	eta;
	edict_t *blip = NULL;

	//gi.dprintf("check_dodge()\n");
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if (G_EntIsAlive(tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->monsterinfo.dodge) 
		&& infront(tr.ent, self) && (tr.endpos[2] > (tr.ent->absmin[2]+abs(tr.ent->mins[2]))))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.aiflags |= AI_DODGE;
		tr.ent->monsterinfo.eta = level.time + eta;
		//gi.dprintf("ETA for impact is %.1f\n", tr.ent->monsterinfo.eta);
		tr.ent->monsterinfo.attacker = self;
		VectorCopy(zvec, self->monsterinfo.dir);
		tr.ent->monsterinfo.radius = 0;
		
	}
	else
	{
		// search for a monster within the radius of the explosion
		while ((blip = findradius(blip, tr.endpos, radius)) != NULL) {
			if (!G_EntIsAlive(blip))
				continue;
			if (!(blip->svflags & SVF_MONSTER))
				continue;
			if (!blip->monsterinfo.dodge)
				continue;
		//	gi.dprintf("found monster\n");
			VectorSubtract(tr.endpos, start, v);
			eta = (VectorLength(v) - blip->maxs[0]) / (float)speed;
			blip->monsterinfo.aiflags |= AI_DODGE;
			blip->monsterinfo.eta = level.time + eta;
			//gi.dprintf("ETA for impact is %.1f\n", tr.ent->monsterinfo.eta);
			blip->monsterinfo.attacker = self;
			VectorCopy(tr.endpos, blip->monsterinfo.dir);
			blip->monsterinfo.radius = radius;	
		}
	}
	
}

/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	if (!self->enemy)
		return false;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	//see if enemy is in range
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->mins[0] && aim[1] < self->maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->enemy->mins[0];
		else
			aim[1] = self->enemy->maxs[0];
	}

	VectorMA (self->s.origin, range, dir, point);

	tr = gi.trace (self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	if (self->enemy)
	{
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
		VectorSubtract (v, point, v);
		VectorNormalize (v);
		VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
		if (self->enemy->velocity[2] > 0)
			self->enemy->groundentity = NULL;
	}
	return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, float damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	//GHz START
	float		dist, temp;
	//GHz END

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

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
			// GHz START
			if (self->client)
			{
				int rangeLevel=0;
				
				// get weapon's range level
				/*if (mod == MOD_SHOTGUN)
					rangeLevel = self->myskills.weapons[WEAPON_SHOTGUN].mods[1].current_level;
				else*/ if (mod == MOD_SSHOTGUN)
					rangeLevel = self->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[1].current_level;
				else rangeLevel = -1; // unknown weapon
				
				if (rangeLevel != -1)
				{
					// get distance from shooter to pellet point of impact
					dist = distance(self->s.origin, tr.endpos);
					
					// pellets begin to drop off at this point
					if (dist > 256)
					{
						temp = dist / (1024.0 + 51.2*rangeLevel);
						if (temp > 1)
							temp = 1;
						//gi.dprintf("%d%c percent chance to miss\n", (int)(temp*100),'%');

						// pellet drops off
						if (random() <= temp)
							return;
					}
				}
			}
			// GHz END

			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, float damage, int kick, int hspread, int vspread, int mod)
{
	//gi.dprintf("DEBUG: bullet fired at %f dealing %d damage\n", level.time,damage);
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, float damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}

void WeaponStun (edict_t *attacker, edict_t *target, int mod)
{
	int		lvl;
	float	duration, value=1.0;

	if (!G_ValidTarget(attacker, target, false))
		return;

	if (!attacker->client)
		return;

	if (attacker->mtype)
		return; // don't allow weapon stun with morphed player weapons

	if (mod == MOD_BLASTER)
	{
		duration = 0.4; // stun duration
		lvl = attacker->myskills.weapons[WEAPON_BLASTER].mods[1].current_level;
		value += 0.033*lvl; // 25% at level 10
	}
	else if (mod == MOD_HYPERBLASTER)
	{
		duration = 0.2;
		lvl = attacker->myskills.weapons[WEAPON_HYPERBLASTER].mods[1].current_level;
		value += 0.033*lvl; // 25% at level 10
	}
	else
		return;

	// cap stun chance to 50%
	if (value > 2)
		value = 2;

	value = 1.0/value;

	if ((random() >= value) 
		&& (level.time+duration > target->holdtime)) // continuous stun is not allowed
	{
		if (target->client)
		{
			// don't stun them if they are invincible
			if (level.framenum > target->client->invincible_framenum)
			{
				target->holdtime = level.time + duration;
				target->client->ability_delay = level.time + duration;
			}
		}
		else
		{
			target->nextthink = level.time + duration;
			target->holdtime = level.time + duration; // doesn't do anything, just need it to keep track of stun time
		}
	}
}

/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/


//K03 Begin
void homing_think (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	//vec3_t  targetdir, blipdir;
	//vec_t   speed;

	//gi.dprintf("%.1f\n", VectorLength(ent->velocity));

	if (ent->delay < level.time)
	{
		G_FreeEdict(ent);
		//K03 Begin
		if (ent->owner && ent->owner->rocket_shots &&
			ent->s.effects & EF_ROCKET)
			ent->owner->rocket_shots--;
		//K03 End
		return;
	}
	
	/*if (ent->owner->bless > level.time)
	{
		while ((blip = findradius(blip, ent->s.origin, 2000)) != NULL)
		{

			if (blip == ent->owner || ent == blip)
				continue;

			if (!blip->takedamage)
				continue;

			if (!(blip->svflags & SVF_MONSTER) && (!blip->client) && (strcmp(blip->classname, "misc_explobox") != 0))
				continue;

			if (!visible(ent, blip))
				continue;
			VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
			blipdir[2] += 16;
			if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
			{
				target = blip;
				VectorCopy(blipdir, targetdir);
			}
		}

		if (target != NULL)
		{
			 // target acquired, nudge our direction toward it
			VectorNormalize(targetdir);
			VectorScale(targetdir, 0.15, targetdir);//was 0.135
			VectorAdd(targetdir, ent->movedir, targetdir);
			VectorNormalize(targetdir);
			VectorCopy(targetdir, ent->movedir);
			vectoangles(targetdir, ent->s.angles);
			speed = VectorLength(ent->velocity);
			VectorScale(targetdir, speed, ent->velocity);

			//is this the first time we locked in? sound warning for the target
			if (ent->homing_lock == 0)
			{
				if (target->client)
					gi.sound (target, CHAN_AUTO, gi.soundindex ("homelock.wav"), 1, ATTN_NORM, 0);
				ent->homing_lock = 1;
			}
		}

		ent->nextthink = level.time + .1;
	}*/
}
//K03 End

void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		WeaponStun(self->owner, other, self->style);
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 0, DAMAGE_ENERGY, self->style);
	}
	else
	{
		if (self->movetype == MOVETYPE_WALLBOUNCE)
		{
			// increment bounce counter
			self->light_level++;
			
			// limit the number of bounces
			if (self->light_level > 2)
				G_FreeEdict(self);

			return;
		}

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

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int proj_type, int mod, float duration, qboolean bounce)
{
	edict_t	*bolt;
	trace_t	tr;

	int movetype, modelindex;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	//gi.dprintf("fire_blaster()\n");

	VectorNormalize (dir);

	bolt = G_Spawn();

	// assign model by projectile type
	if (proj_type == BLASTER_PROJ_BLAST)
	{
		modelindex = gi.modelindex ("models/objects/flball/tris.md2");
		VectorSet (bolt->mins, -8, -8, 8);
		VectorSet (bolt->maxs, 8, 8, 8);
	}
	else if (proj_type == BLASTER_PROJ_BOLT)
	{
		modelindex = gi.modelindex ("models/objects/laser/tris.md2");
		VectorClear (bolt->mins);
		VectorClear (bolt->maxs);
	}
	else
	{
		modelindex = gi.modelindex ("models/objects/laser/tris.md2");
		VectorClear (bolt->mins);
		VectorClear (bolt->maxs);
	}

	/*if (effect == EF_BLUEHYPERBLASTER)
		modelindex = gi.modelindex("models/objects/blaser/tris.md2");*/

	// select movetype
	if (bounce)
		movetype = MOVETYPE_WALLBOUNCE;
	else
		movetype = MOVETYPE_FLYMISSILE;

	bolt->s.modelindex = modelindex;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = movetype;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->style = mod;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 9;// duration; // 2016 SLOW
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	gi.linkentity (bolt);

	// call monster's dodge function
	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed, 0);

	// see if our shot is blocked
	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	


/*
=================
fire_grenade
=================
*/
void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	// remove grenade if owner dies or becomes invalid
	if (!G_EntValid(ent->owner))
	{
		BecomeExplosion1(ent);
		return;
	}

	if (ent->owner->client && !(ent->owner->svflags & SVF_DEADMONSTER))
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		//points = ent->dmg - 0.5 * VectorLength (v);
		// (apple)
		// Hit the target for full damage on direct hit.
		points = ent->dmg;
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;

		if (ent->spawnflags & 4) //GHz
			mod = MOD_BOMBS;

		if (ent->mtype == M_MIRV)
			mod = MOD_MIRV;

		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;

	if (ent->spawnflags & 4) //GHz
			mod = MOD_BOMBS;

	if (ent->mtype == M_MIRV)
		mod = MOD_MIRV;

	//K03 Begin
	if (Q_stricmp(ent->classname, "grenade") == 0)
	{
	//	ent->dmg = GRENADELAUNCHER_INITIAL_DAMAGE + GRENADELAUNCHER_ADDON_DAMAGE * ent->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level) / 2;
	//	gi.dprintf("ent->dmg %d\n", ent->dmg);
		if (is_quad)
			ent->dmg *= 4;
		//gi.dprintf("DEBUG: spawnflags %d\n", ent->spawnflags);
		if (/*mod != MOD_BOMBS && mod != MOD_HANDGRENADE && mod != MOD_HG_SPLASH && */(ent->spawnflags & 9))//GHz
			ent->owner->max_pipes--;

		if (ent->owner->max_pipes < 0)
			ent->owner->max_pipes = 0;
	}
	//K03 End

	// (apple)
	// Deal the specified radius damage instead
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent->enemy, ent->dmg_radius, mod);

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
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Cluster_Explode (edict_t *ent)

{
	vec3_t		origin;

	//Sean added these 4 vectors

	vec3_t   grenade1;
	vec3_t   grenade2;
	vec3_t   grenade3;
	vec3_t   grenade4;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	ent->owner->max_pipes--;
	if (ent->owner->max_pipes < 0)
		ent->owner->max_pipes = 0;

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_G_SPLASH);

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

	// Sean : explode the four grenades outwards
//	void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
	fire_grenade2(ent->owner, origin, grenade1, ent->dmg, 10, 1.5, (ent->dmg_radius*0.5), ent->radius_dmg, false);
	fire_grenade2(ent->owner, origin, grenade2, ent->dmg, 10, 1.5, (ent->dmg_radius*0.5), ent->radius_dmg, false);
	fire_grenade2(ent->owner, origin, grenade3, ent->dmg, 10, 1.5, (ent->dmg_radius*0.5), ent->radius_dmg, false);
	fire_grenade2(ent->owner, origin, grenade4, ent->dmg, 10, 1.5, (ent->dmg_radius*0.5), ent->radius_dmg, false);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	ent->enemy = NULL;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		if (ent->spawnflags & 9)
		{
			ent->owner->max_pipes--;
			if (ent->owner->max_pipes < 0)
				ent->owner->max_pipes = 0;
		}
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
		//GHz: Stick pipebombs to wall
		//gi.dprintf("DEBUG: spawnflags %d\n", ent->spawnflags);
		if (ent->spawnflags == 9)
		{
			VectorClear (ent->velocity) ;
			VectorClear (ent->avelocity) ;
			ent->movetype = MOVETYPE_NONE;
		}

		return;
	}

	// pipe bombs are not activated by teammates
	if ((ent->spawnflags == 9) && G_EntIsAlive(other) 
		&& G_EntIsAlive(ent->owner) && OnSameTeam(ent->owner, other))
		return;
	//{
		ent->enemy = other;
		Grenade_Explode (ent);
	//}
	//Cluster_Explode(ent);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int radius_damage)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	//K03 Begin
	if ((self->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[3].current_level < 1) || !self->client)
		grenade->s.effects |= EF_GRENADE;
	//K03 End
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	//K03 Begin
	if (self->svflags & SVF_MONSTER)
		grenade->nextthink = level.time + timer;
	else
	{
		if (self->client && self->client->weapon_mode)//GHz
			grenade->nextthink = level.time + 999;
		else
			grenade->nextthink = level.time + timer;
	}
	if (self->client && self->client->weapon_mode)
		grenade->spawnflags = 9;//GHz: Pipe bomb fired or not?
	//K03 End
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->radius_dmg = radius_damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	gi.linkentity (grenade);
}

edict_t *fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, int radius_damage, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	//K03 Begin
	if ((self->myskills.weapons[WEAPON_HANDGRENADE].mods[3].current_level < 1) || !self->client)
		grenade->s.effects |= EF_GRENADE;
	//K03 End
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	// (apple)
	// All grenades deal damage depending on whether
	// it's a direct hit or not, similar to RL.
	grenade->radius_dmg = radius_damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}

	return grenade;
}

/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	//K03 Begin
	int mod=MOD_ROCKET;
	if (Q_stricmp(ent->owner->classname, "SentryGun") == 0)
		mod = MOD_SENTRY_ROCKET;
	if (Q_stricmp(ent->owner->classname, "msentrygun") == 0)
		mod = MOD_SENTRY_ROCKET;
	if (Q_stricmp(ent->owner->classname, "Sentry_Gun") == 0)
		mod = MOD_SENTRY_ROCKET;

	if (!G_EntExists(ent->owner))
	{
		G_FreeEdict(ent);
		return;
	}

	if (other == ent->owner)
		return;
	//K03 End

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, mod);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

// GHz START
// v3.03 bug catcher
// calling t_damage() can cause damage to attacker, so make sure he is still valid
	if (!G_EntExists(ent->owner))
	{
		gi.dprintf("WARNING: rocket_touch() has no valid owner\n");
		return;
	}
//GHz END


	//K03 Begin
	if ((!ent->owner->client) && (mod != MOD_SENTRY_ROCKET))
		mod = MOD_R_SPLASH;

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, mod);
	//K03 End

	/*if(Q_stricmp (ent->classname, "lockon rocket") == 0)
		gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/locrexp.wav"), 1, ATTN_NONE, 0);*/

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void rocket_think (edict_t *self)
{
	// remove rocket if owner dies or becomes invalid
	// or the rocket times out
	if (!G_EntValid(self->owner) || (level.time >= self->delay))
	{
		BecomeExplosion1(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//K03 Begin
	if ((self->client && self->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[3].current_level < 1) || !self->client)
		rocket->s.effects |= EF_ROCKET;
	//K03 End
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	//K03 Begin
	rocket->nextthink = level.time + FRAMETIME;
	rocket->think = rocket_think;// GHz v3.1
	rocket->delay = level.time + 8000/speed;
	//rocket->think = G_FreeEdict;
	//K03 End
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed, damage_radius);

	gi.linkentity (rocket);
}

void think_lockon_rocket(edict_t *ent)
{
	vec3_t	v;

	if(ent->moveinfo.speed < 100)
	{
		ent->s.sound = gi.soundindex ("3zb/locrfly.wav");
		ent->moveinfo.speed = 100;
	}

	if(ent->moveinfo.speed < 1600) ent->moveinfo.speed *= 2;

	if(ent->target_ent)
	{
		if(!ent->target_ent->inuse || ent->target_ent->deadflag)
		{
			ent->target_ent = NULL;
			ent->movetype = MOVETYPE_BOUNCE;
			ent->touch = Grenade_Touch;
			ent->think = Grenade_Explode;
			ent->nextthink = level.time + FRAMETIME * 15;
			ent->s.sound = 0;

			VectorCopy(ent->velocity,v);
			VectorNormalize(v);
			VectorScale (v, ent->moveinfo.speed, ent->velocity);	
			return;
		}
		else
		{
			VectorSubtract(ent->target_ent->s.origin,ent->s.origin,v);
			VectorNormalize(v);
			vectoangles (v, ent->s.angles);
			VectorScale (v, ent->moveinfo.speed, ent->velocity);			
			ent->nextthink = level.time + FRAMETIME ;//* 2.0;
		}
	}
	else
	{
		ent->movetype = MOVETYPE_BOUNCE;
		ent->touch = Grenade_Touch;
		ent->think = Grenade_Explode;
		ent->nextthink = level.time + FRAMETIME * 15;
		ent->s.sound = 0;

		VectorCopy(ent->velocity,v);
		VectorNormalize(v);
		VectorScale (v, ent->moveinfo.speed, ent->velocity);			
		return;
	}

	//ŽžŠÔØ‚ê
	if(ent->moveinfo.accel <= level.time) 
	{
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, MOD_R_SPLASH);

		gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/locrexp.wav"), 1, ATTN_NONE, 0);
		gi.WriteByte (svc_temp_entity);
		if (ent->waterlevel)
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PHS);

		G_FreeEdict (ent);	
	}
}
void fire_lockon_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);

	rocket->moveinfo.speed = speed;

	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;

	rocket->nextthink = level.time + FRAMETIME * 8;

	rocket->moveinfo.accel = level.time + FRAMETIME * 36;

	rocket->think = think_lockon_rocket;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "lockon rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed, damage_radius);

	gi.linkentity (rocket);
}

#define SMARTROCKET_TIMEOUT	5

void smartrocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	gi.sound(ent, CHAN_VOICE, gi.soundindex ("weapons/srocket_explode.wav"), 1, ATTN_NORM, 0);

	// do direct damage
	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
	// do radius damage
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);
	BecomeExplosion1(ent);
}

void smartrocket_lockon (edict_t *self)
{
	vec3_t	v, forward;

	// modify angles to point to enemy
	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	VectorNormalize(v);
	VectorScale(v, self->random, v);
	VectorAdd(v, self->movedir, v);
	VectorNormalize(v);
	VectorCopy(v, self->movedir);
	vectoangles(v, self->s.angles);

	//gi.dprintf("yaw %d at frame %d (%.1f)\n", (int)self->s.angles[YAW], level.framenum, self->random);

	// move in the direction we are facing
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorScale (forward, self->count, self->velocity);
}

void smartrocket_think (edict_t *self)
{
	// remove rocket if owner dies or becomes invalid
	// or the rocket times out
	if (!G_EntIsAlive(self->owner) || (level.time >= self->delay))
	{
		BecomeExplosion1(self);
		return;
	}

	if (G_ValidTarget(self, self->enemy, true))
		smartrocket_lockon(self);
	
	self->nextthink = level.time + FRAMETIME;
}

void fire_smartrocket (edict_t *self, edict_t *target, vec3_t start, vec3_t dir, int damage, 
					   int speed, int turn_speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->s.effects |= EF_ROCKET;
	rocket->owner = self;
	rocket->touch = smartrocket_touch;
	rocket->nextthink = level.time + FRAMETIME * 3; // short delay before intelligence kicks-in
	rocket->think = smartrocket_think;// GHz v3.1
	rocket->delay = level.time + SMARTROCKET_TIMEOUT;
	rocket->count = speed;
	rocket->dmg = damage;

	// has a target lock been established?
	if (turn_speed >= SMARTROCKET_LOCKFRAMES)
	{
		rocket->enemy = target;

		// adjust turn rate based on quality of lock
		// 0.1 = 5-6 degrees/frame, 0.2 = 11-12, 0.3 = 17-18, 0.4 = 23-24
		rocket->random = 0.1 * (turn_speed - (SMARTROCKET_LOCKFRAMES - 1));
		
		// maximum turning rate
		if (rocket->random > 1.0)
			rocket->random = 1.0;
	}

	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "smartrocket";
	gi.linkentity (rocket);
}

/*
=================
fire_rail
=================
*/
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;
	int			mod;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
			{
				// special MOD for sniper shot
				if (self->client && self->client->weapon_mode)
					mod = MOD_SNIPER;
				else
					mod = MOD_RAILGUN;
				// only cause burn if damage is actually inflicted
				if (T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_PIERCING, mod))
				{
					if (self->myskills.weapons[WEAPON_RAILGUN].mods[2].current_level > 0)
						burn_person(tr.ent, self, (int)(RAILGUN_ADDON_HEATDAMAGE * self->myskills.weapons[WEAPON_RAILGUN].mods[2].current_level));
				}
				
				// stop on sentry
				if (tr.ent->mtype == M_SENTRY)
					break;
			}
		}

		VectorCopy (tr.endpos, from);
	}

	if (!self->client || particles->value || (!self->client->weapon_mode 
		&& (self->myskills.weapons[WEAPON_RAILGUN].mods[3].current_level < 1)))
	{
		if (self->myskills.weapons[WEAPON_RAILGUN].mods[0].current_level >= 5)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos);
			gi.multicast (self->s.origin, MULTICAST_PHS);
			
		}
		// send gun puff / flash
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	//	gi.multicast (start, MULTICAST_PHS);
		if (water)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_RAILTRAIL);
			gi.WritePosition (start);
			gi.WritePosition (tr.endpos);
			gi.multicast (tr.endpos, MULTICAST_PHS);
		}
	}

	if (self->client && self->myskills.weapons[WEAPON_RAILGUN].mods[4].current_level < 1)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

/*
=================
fire_20mm
=================
*/
void fire_20mm (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	vec3_t		forward, right, start1, offset;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water, ducked;

	int range = WEAPON_20MM_INITIAL_RANGE + (WEAPON_20MM_ADDON_RANGE * self->myskills.weapons[WEAPON_20MM].mods[1].current_level);

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

//	gi.dprintf("fire_20mm called at %.1f\n", level.time);

	VectorMA (start, range, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_PIERCING, MOD_CANNON);
				//if (self->myskills.weapons[WEAPON_RAILGUN].mods[2].current_level > 0)//K03
					//burn_person(tr.ent, self, (int)(RAILGUN_ADDON_HEATDAMAGE * self->myskills.weapons[WEAPON_RAILGUN].mods[2].current_level));
			}
		}

		VectorCopy (tr.endpos, from);

		if (tr.fraction != 1.0) // vrc 2.32: give some feedback
		{
			if (strncmp (tr.surface->name, "sky", 3) != 0)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_GUNSHOT);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.multicast (tr.endpos, MULTICAST_PVS);

				if (self->client)
					PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
			}
		}

	//GHz Start
		if (!self->client || (self->client->ps.pmove.pm_flags & PMF_DUCKED))
			ducked = true;
		else
			ducked = false;

		if (ducked == false)
		{
			AngleVectors (self->client->v_angle, forward, right, NULL);

			VectorScale (forward, -3, self->client->kick_origin);

			VectorSet(offset, 0, 7,  self->viewheight-8);

			P_ProjectSource (self->client, self->s.origin, offset, forward, right, start1);

			self->velocity[0] -= forward[0] * (500 - (25 * self->myskills.weapons[WEAPON_20MM].mods[2].current_level));
			self->velocity[1] -= forward[1] * (500 - (25 * self->myskills.weapons[WEAPON_20MM].mods[2].current_level));
		}
	//GHz End

	}

}

void fire_sniperail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy (tr.endpos, from);
	}

	VectorScale (aimdir,100, from);
	VectorSubtract(tr.endpos,from,start);

//	gi.bprintf(PRINT_HIGH,"jj\n");

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

/*
=================
fire_bfg
=================
*/
void bfg_explode (edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->owner))
				continue;

			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	// stop moving if slide isn't upgraded or we hit something we can hurt
	if (!self->radius_dmg || (other && other->inuse && other->takedamage))
		VectorClear(self->velocity);
	else
	{
		float oldVelocity, newVelocity;

		// bfg slows down when it touches something
		oldVelocity = VectorLength(self->velocity);
		newVelocity = (2 * BFG10K_INITIAL_SPEED) - oldVelocity;

		// don't stop completely
		if (newVelocity < 100)
			newVelocity = 100;

		// don't go faster than we are already going
		if (newVelocity > oldVelocity)
			newVelocity = oldVelocity;

		// apply new velocity
		VectorNormalize(self->velocity);
		VectorScale(self->velocity, newVelocity, self->velocity);
	}

	// if we registered a hit, then dont update delay
	if (self->style)
		return;

	self->style = 1;// we hit something
	self->delay = level.time + self->random;
	self->think = bfg_think;

	// call think function immediately so we can cause some damage
	// otherwise, we will miss a frame of damage
	self->think(self);
}

void bfg_think (edict_t *self)
{
	int			damage, range, distance;
	edict_t		*target = NULL;
	vec3_t		dir, v;
	trace_t		tr;

	// owner must be alive
	if (!G_EntIsAlive(self->owner) || (self->delay <= level.time)) 
	{
		G_FreeEdict(self);
		return;
	}

	damage = self->dmg;
	range = self->dmg_radius;

	// find a valid target
	while ((target = findradius(target, self->s.origin, range)) != NULL)
	{
		if (target == self)
			continue;
		if (!G_EntExists(target))
			continue;
		if (target->client && (target->client->respawn_time > level.time))
			continue;
		if (OnSameTeam(self->owner, target) && (target != self->owner))
			continue;
		if (!visible1(self, target))
			continue;

		// dont trap frozen targets
		if (que_typeexists(target->curses, CURSE_FROZEN))
		{
			G_FreeEdict(self);
			return;
		}

		VectorSubtract(target->s.origin, self->s.origin, dir);
		VectorNormalize(dir);

		// deal damage based on target distance
		tr = gi.trace(self->s.origin, NULL, NULL, target->s.origin, self, MASK_SHOT);
		VectorSubtract(tr.endpos, self->s.origin, v);
		distance = VectorLength(v);
		damage *= 1 - ( (float) distance / (float) range );
		if (target == self->owner)
			damage *= 0.5;
		
		T_Damage (target, self, self->owner, dir, tr.endpos, vec3_origin, damage, 0, DAMAGE_ENERGY, MOD_BFG_LASER);

		// bfg pull
		/*VectorNormalize(v);

		VectorScale(v, self->owner->myskills.weapons[WEAPON_BFG10K].mods[4].current_level*5, v);

		VectorSubtract(target->s.origin, v, v);

		tr = gi.trace(v, target->mins, target->maxs, v, target->s.origin, MASK_PLAYERSOLID);

		if (tr.fraction == 1.0) // we hit nothing! :D
		{
			VectorCopy(v, target->s.origin);
			gi.linkentity(target);
		}*/
		
		//gi.dprintf("BFG caused %d damage at %.1f\n", damage, level.time);

		// bfg laser effect
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
}

void bfg_idle_think (edict_t *self)
{
	// owner must be alive
	if (!G_EntIsAlive(self->owner) || (self->delay < level.time)) 
	{
		G_FreeEdict(self);
		return;
	}

	self->nextthink = level.time + FRAMETIME;
}

void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_SLIDE;//MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
	bfg->delay = level.time + 10.0; // initial time to expire

	if (self->client)
	{
		// set the delay for when the BFG will expire after hitting something
		bfg->random = BFG10K_INITIAL_DURATION + BFG10K_ADDON_DURATION * self->myskills.weapons[WEAPON_BFG10K].mods[1].current_level;
		// slide upgrade
		bfg->radius_dmg = self->myskills.weapons[WEAPON_BFG10K].mods[3].current_level;
	}
	else
	{
		bfg->random = BFG10K_DEFAULT_DURATION;
		bfg->radius_dmg = BFG10K_DEFAULT_SLIDE;
	}

	bfg->think = bfg_idle_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;
	if (self->client)
		check_dodge (self, bfg->s.origin, dir, speed, damage_radius);
	gi.linkentity (bfg);
}

/*===============================================================================*/

// RAFAEL
/*
=================
	fire_ionripper
=================
*/

void ionripper_sparks (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_WELDING_SPARKS);
	gi.WriteByte (0);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.WriteByte (0xe4 + (rand()&3));
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}

// RAFAEL
void ionripper_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
			PlayerNoise (self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_RIPPER);

	}
	else
	{
		return;
	}

	G_FreeEdict (self);
}


// RAFAEL
void fire_ionripper (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t *ion;
	trace_t tr;

	VectorNormalize (dir);

	ion = G_Spawn ();
	VectorCopy (start, ion->s.origin);
	VectorCopy (start, ion->s.old_origin);
	vectoangles (dir, ion->s.angles);
	VectorScale (dir, speed, ion->velocity);

	ion->movetype = MOVETYPE_WALLBOUNCE;
	ion->clipmask = MASK_SHOT;
	ion->solid = SOLID_BBOX;
	ion->s.effects |= effect;

	ion->s.renderfx |= RF_FULLBRIGHT;

	VectorClear (ion->mins);
	VectorClear (ion->maxs);
	ion->s.modelindex = gi.modelindex ("models/objects/boomrang/tris.md2");
	ion->s.sound = gi.soundindex ("misc/lasfly.wav");
	ion->owner = self;
	ion->touch = ionripper_touch;
	ion->nextthink = level.time + 3;
	ion->think = ionripper_sparks;
	ion->dmg = damage;
	ion->dmg_radius = 100;
	gi.linkentity (ion);

	if (self->client)
		check_dodge (self, ion->s.origin, dir, speed, 100);

	tr = gi.trace (self->s.origin, NULL, NULL, ion->s.origin, ion, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (ion->s.origin, -10, dir, ion->s.origin);
		ion->touch (ion, tr.ent, NULL, NULL);
	}

}


// RAFAEL
/*
=================
fire_heat
=================
*/
/*
void heat_think (edict_t *self)
{
	edict_t		*target = NULL;
	edict_t		*aquire = NULL;
	vec3_t		vec;
	vec3_t		oldang;
	int			len;
	int			oldlen = 0;

	VectorClear (vec);

	// aquire new target
	while (( target = findradius (target, self->s.origin, 1024)) != NULL)
	{
		
		if (self->owner == target)
			continue;
		if (!target->svflags & SVF_MONSTER)
			continue;
		if (!target->client)
			continue;
		if (target->health <= 0)
			continue;
		if (!visible (self, target))
			continue;
		
		// if we need to reduce the tracking cone
		/*
		{
			vec3_t	vec;
			float	dot;
			vec3_t	forward;
	
			AngleVectors (self->s.angles, forward, NULL, NULL);
			VectorSubtract (target->s.origin, self->s.origin, vec);
			VectorNormalize (vec);
			dot = DotProduct (vec, forward);
	
			if (dot > 0.6)
				continue;
		}
		*/

/*		if (!infront (self, target))
			continue;

		VectorSubtract (self->s.origin, target->s.origin, vec);
		len = VectorLength (vec);

		if (aquire == NULL || len < oldlen)
		{
			aquire = target;
			self->target_ent = aquire;
			oldlen = len;
		}
	}

	if (aquire != NULL)
	{
		VectorCopy (self->s.angles, oldang);
		VectorSubtract (aquire->s.origin, self->s.origin, vec);
		
		vectoangles (vec, self->s.angles);
		
		VectorNormalize (vec);
		VectorCopy (vec, self->movedir);
		VectorScale (vec, 500, self->velocity);
	}

	self->nextthink = level.time + 0.1;
}
*/
// RAFAEL
/*
void fire_heat (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *heat;

	heat = G_Spawn();
	VectorCopy (start, heat->s.origin);
	VectorCopy (dir, heat->movedir);
	vectoangles (dir, heat->s.angles);
	VectorScale (dir, speed, heat->velocity);
	heat->movetype = MOVETYPE_FLYMISSILE;
	heat->clipmask = MASK_SHOT;
	heat->solid = SOLID_BBOX;
	heat->s.effects |= EF_ROCKET;
	VectorClear (heat->mins);
	VectorClear (heat->maxs);
	heat->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	heat->owner = self;
	heat->touch = rocket_touch;

	heat->nextthink = level.time + 0.1;
	heat->think = heat_think;
	
	heat->dmg = damage;
	heat->radius_dmg = radius_damage;
	heat->dmg_radius = damage_radius;
	heat->s.sound = gi.soundindex ("weapons/rockfly.wav");

	if (self->client)
		check_dodge (self, heat->s.origin, dir, speed);

	gi.linkentity (heat);
}
*/


// RAFAEL
/*
=================
	fire_plasma
=================
*/

void plasma_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_PHALANX);
	}
	
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_PHALANX);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PLASMA_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	G_FreeEdict (ent);
}


// RAFAEL
void fire_plasma (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t *plasma;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	plasma = G_Spawn();
	VectorCopy (start, plasma->s.origin);
	VectorCopy (dir, plasma->movedir);
	vectoangles (dir, plasma->s.angles);
	VectorScale (dir, speed, plasma->velocity);
	plasma->movetype = MOVETYPE_FLYMISSILE;
	plasma->clipmask = MASK_SHOT;
	plasma->solid = SOLID_BBOX;

	VectorClear (plasma->mins);
	VectorClear (plasma->maxs);
	
	plasma->owner = self;
	plasma->touch = plasma_touch;
	plasma->nextthink = level.time + 8000/speed;
	plasma->think = G_FreeEdict;
	plasma->dmg = damage;
	plasma->radius_dmg = radius_damage;
	plasma->dmg_radius = damage_radius;
	plasma->s.sound = gi.soundindex ("weapons/rockfly.wav");
	
	plasma->s.modelindex = gi.modelindex ("sprites/s_photon.sp2");
	plasma->s.effects |= EF_PLASMA | EF_ANIM_ALLFAST;
	
	if (self->client)
		check_dodge (self, plasma->s.origin, dir, speed, damage_radius);

	gi.linkentity (plasma);

	
}


/*
=================
trap
=================
*/

// RAFAEL
extern void SP_item_foodcube (edict_t *best);
// RAFAEL
static void Trap_Think (edict_t *ent)
{
	edict_t	*target = NULL;
	edict_t	*best = NULL;
	vec3_t	vec;
	int		len, i;
	int		oldlen = 8000;
	vec3_t	forward, right, up;
	
	if (ent->timestamp < level.time)
	{
		BecomeExplosion1(ent);
		// note to self
		// cause explosion damage???
		return;
	}
	
	ent->nextthink = level.time + 0.1;
	
	if (!ent->groundentity)
		return;

	// ok lets do the blood effect
	if (ent->s.frame > 4)
	{
		if (ent->s.frame == 5)
		{
			if (ent->wait == 64)
				gi.sound(ent, CHAN_VOICE, gi.soundindex ("weapons/trapdown.wav"), 1, ATTN_IDLE, 0);

			ent->wait -= 2;
			ent->delay += level.time;

			for (i=0; i<3; i++)
			{
				
				best = G_Spawn();

				if (strcmp (ent->enemy->classname, "monster_gekk") == 0)
				{
					best->s.modelindex = gi.modelindex ("models/objects/gekkgib/torso/tris.md2");	
					best->s.effects |= TE_GREENBLOOD;
				}
				else if (ent->mass > 200)
				{
					best->s.modelindex = gi.modelindex ("models/objects/gibs/chest/tris.md2");	
					best->s.effects |= TE_BLOOD;
				}
				else
				{
					best->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");	
					best->s.effects |= TE_BLOOD;
				}

				AngleVectors (ent->s.angles, forward, right, up);
				
				RotatePointAroundVector( vec, up, right, ((360.0/3)* i)+ent->delay);
				VectorMA (vec, ent->wait/2, vec, vec);
				VectorAdd(vec, ent->s.origin, vec);
				VectorAdd(vec, forward, best->s.origin);
  
				best->s.origin[2] = ent->s.origin[2] + ent->wait;
				
				VectorCopy (ent->s.angles, best->s.angles);
  
				best->solid = SOLID_NOT;
				best->s.effects |= EF_GIB;
				best->takedamage = DAMAGE_YES;
		
				best->movetype = MOVETYPE_TOSS;
				best->svflags |= SVF_MONSTER;
				best->deadflag = DEAD_DEAD;
		  
				VectorClear (best->mins);
				VectorClear (best->maxs);

				best->watertype = gi.pointcontents(best->s.origin);
				if (best->watertype & MASK_WATER)
					best->waterlevel = 1;

				best->nextthink = level.time + 0.1;
				best->think = G_FreeEdict;
				gi.linkentity (best);
			}
				
			if (ent->wait < 19)
				ent->s.frame ++;

			return;
		}
		ent->s.frame ++;
		if (ent->s.frame == 8)
		{
			ent->nextthink = level.time + 1.0;
			ent->think = G_FreeEdict;

			best = G_Spawn ();
			SP_item_foodcube (best);
			VectorCopy (ent->s.origin, best->s.origin);
			best->s.origin[2]+= 16;
			best->velocity[2] = 400;
			best->count = ent->mass;
			gi.linkentity (best);
			return;
		}
		return;
	}
	
	ent->s.effects &= ~EF_TRAP;
	if (ent->s.frame >= 4)
	{
		ent->s.effects |= EF_TRAP;
		VectorClear (ent->mins);
		VectorClear (ent->maxs);

	}

	if (ent->s.frame < 4)
		ent->s.frame++;

	while ((target = findradius(target, ent->s.origin, 256)) != NULL)
	{
		if (target == ent)
			continue;
		if (!(target->svflags & SVF_MONSTER) && !target->client)
			continue;
		// if (target == ent->owner)
		//	continue;
		if (target->health <= 0)
			continue;
		if (!visible (ent, target))
		 	continue;
		if (!best)
		{
			best = target;
			continue;
		}
		VectorSubtract (ent->s.origin, target->s.origin, vec);
		len = VectorLength (vec);
		if (len < oldlen)
		{
			oldlen = len;
			best = target;
		}
	}

	// pull the enemy in
	if (best)
	{
		vec3_t	forward;

		if (best->groundentity)
		{
			best->s.origin[2] += 1;
			best->groundentity = NULL;
		}
		VectorSubtract (ent->s.origin, best->s.origin, vec);
		len = VectorLength (vec);
		if (best->client)
		{
			VectorNormalize (vec);
			VectorMA (best->velocity, 250, vec, best->velocity);
		}
		else
		{
			best->ideal_yaw = vectoyaw(vec);	
//			M_ChangeYaw (best);
			AngleVectors (best->s.angles, forward, NULL, NULL);
			VectorScale (forward, 256, best->velocity);
		}

		gi.sound(ent, CHAN_VOICE, gi.soundindex ("weapons/trapsuck.wav"), 1, ATTN_IDLE, 0);
		
		if (len < 32)
		{	
			if (best->mass < 400)
			{
				T_Damage (best, ent, ent->owner, vec3_origin, best->s.origin, vec3_origin, 100000, 1, 0, MOD_TRAP);
				ent->enemy = best;
				ent->wait = 64;
				VectorCopy (ent->s.origin, ent->s.old_origin);
				ent->timestamp = level.time + 30;
				if (deathmatch->value)
					ent->mass = best->mass/4;
				else
					ent->mass = best->mass/10;
				// ok spawn the food cube
				ent->s.frame = 5;	
			}
			else
			{
				BecomeExplosion1(ent);
				// note to self
				// cause explosion damage???
				return;
			}
				
		}
	}

		
}

void spawn_grenades(edict_t *ent, vec3_t origin, float time, int damage, int num) 
{
	int		j;
	edict_t *grenade;

  for (j=1; j<=num; j++) {
    grenade = G_Spawn();
    grenade->owner = ent;
    grenade->s.modelindex = gi.modelindex("models/objects/grenade/tris.md2");
    VectorCopy(origin,grenade->s.origin);
    VectorSet(grenade->velocity,0,0,-300);
    VectorSet(grenade->avelocity,200+crandom()*10.0,200+crandom()*10.0,200+crandom()*10.0);
    grenade->movetype = MOVETYPE_BOUNCE;
    grenade->classname = "grenade";
    grenade->spawnflags = 4;
    grenade->clipmask = MASK_SHOT;
    grenade->solid=SOLID_BBOX;
    grenade->s.effects |= EF_GRENADE;
	grenade->dmg = damage;
    grenade->dmg_radius = 100;
	grenade->radius_dmg = damage;
    VectorSet(grenade->mins,-8,-8,-8);
    VectorSet(grenade->maxs,8,8,8);
    grenade->touch = Grenade_Touch;
    grenade->think = Grenade_Explode;
    grenade->nextthink = level.time + time;
    gi.linkentity(grenade);
  }
}

#define EMP_BULLET_FACTOR				0.5
#define EMP_CELL_FACTOR					0.5
#define EMP_SHELL_FACTOR				1
#define EMP_ROCKET_FACTOR				2
#define EMP_GRENADE_FACTOR				2
#define EMP_SLUG_FACTOR					2
#define EMP_MIN_RADIUS					100
#define EMP_MAX_RADIUS					300
#define EMP_MAX_DAMAGE					1000
#define EMP_INITIAL_AMMO				0
#define EMP_ADDON_AMMO					0.05	// 5% ammo per level is detonated
#define EMP_COST						25
#define EMP_DELAY						0.5
#define EMP_INITIAL_TIME				0
#define EMP_ADDON_TIME					0.5
#define EMP_INITIAL_RADIUS				150
#define EMP_ADDON_RADIUS				0
#define EMP_AMMOBOX_INITIAL_DAMAGE		50
#define EMP_AMMOBOX_ADDON_DAMAGE		5

void EmpEffects (edict_t *ent)
{
	vec3_t	start, dir;
	int		i=GetRandom(0, 2);

	start[i] = ent->absmax[i];
	i++;
	if (i>2)
		i=0;
	start[i] = GetRandom((int) ent->absmin[i], (int) ent->absmax[i]);
	i++;
	if (i>2)
		i=0;
	start[i] = GetRandom((int) ent->absmin[i], (int) ent->absmax[i]);

	VectorSubtract(ent->s.origin, start, dir);
	VectorNormalize(dir);

	// throw some sparks around the entities bounding box
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_LASER_SPARKS);
	gi.WriteByte(GetRandom(5, 15)); // number of sparks
	gi.WritePosition(start);
	gi.WriteDir(dir);
	gi.WriteByte(210); // 242 = red, 210 = green, 2 = black
	gi.multicast(start, MULTICAST_PVS);

	// make powered armor flash on and off

	
	// make powered armor flash on and off
	if (!ctf->value && !domination->value && ent->monsterinfo.power_armor_power)
	{
		if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects ^= EF_COLOR_SHELL;
			ent->s.renderfx ^= RF_SHELL_GREEN;
		}
		else
		{
			ent->s.effects ^= EF_POWERSCREEN;
		}
	}

}

void ExplodeAmmo (edict_t *ent, edict_t *other)
{
	int		dmg=0, qty, use;
	float	rad, initial=EMP_INITIAL_AMMO, addon=EMP_ADDON_AMMO;

	// 200-1200 max bullets
	qty = other->client->pers.inventory[bullet_index];
	use = (initial + addon * ent->monsterinfo.level) * MAX_BULLETS(other);
	if (use > qty)
		use = qty;
	dmg += EMP_BULLET_FACTOR*use;
	other->client->pers.inventory[bullet_index] -= use;
	//gi.dprintf("used %d of %d bullets\n", use, qty);

	// 200-1200 max cells
	qty = other->client->pers.inventory[cell_index];
	use = (initial + addon * ent->monsterinfo.level) * MAX_CELLS(other);
	if (use > qty)
		use = qty;
	dmg += EMP_CELL_FACTOR*use;
	other->client->pers.inventory[cell_index] -= use;
	//gi.dprintf("used %d of %d cells\n", use, qty);

	// 100-600 max shells
	qty = other->client->pers.inventory[shell_index];
	use = (initial + addon * ent->monsterinfo.level) * MAX_SHELLS(other);
	if (use > qty)
		use = qty;
	dmg += EMP_SHELL_FACTOR*use;
	other->client->pers.inventory[shell_index] -= use;
	//gi.dprintf("used %d of %d shells\n", use, qty);

	// 50-300 max rockets
	qty = other->client->pers.inventory[rocket_index];
	use = (initial + addon * ent->monsterinfo.level) * MAX_ROCKETS(other);
	if (use > qty)
		use = qty;
	dmg += EMP_ROCKET_FACTOR*use;
	other->client->pers.inventory[rocket_index] -= use;
	//gi.dprintf("used %d of %d rockets\n", use, qty);

	// 50-300 max grenades
	qty = other->client->pers.inventory[grenade_index];
	use = (initial + addon * ent->monsterinfo.level) * MAX_GRENADES(other);
	if (use > qty)
		use = qty;
	dmg += EMP_GRENADE_FACTOR*use;
	other->client->pers.inventory[grenade_index] -= use;
	//gi.dprintf("used %d of %d grenades\n", use, qty);

	// 50-300 max slugs
	qty = other->client->pers.inventory[slug_index];
	use = (initial + addon * ent->monsterinfo.level) * MAX_SLUGS(other);
	if (use > qty)
		use = qty;
	dmg += EMP_SLUG_FACTOR*use;
	other->client->pers.inventory[slug_index] -= use;
	//gi.dprintf("used %d of %d slugs\n", use, qty);

	//gi.dprintf("damage = %d\n", dmg);

	if (dmg > 0)
	{
		if (dmg > EMP_MAX_DAMAGE)
			dmg = EMP_MAX_DAMAGE;

		// hurt them
		T_Damage(other, ent, ent->owner, vec3_origin, other->s.origin, vec3_origin, dmg, 0, 0, MOD_EMP);

		// hurt others from exploding ammo
		rad = 0.5*dmg;
		if (rad < EMP_MIN_RADIUS)
			rad = EMP_MIN_RADIUS;
		if (rad > EMP_MAX_RADIUS)
			rad = EMP_MAX_RADIUS;
		T_RadiusDamage(ent, ent->owner, dmg, other, rad, MOD_EMP);

		// teleport effect to indicate exploding ammo
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_TELEPORT_EFFECT);
		gi.WritePosition (other->s.origin);
		gi.multicast (other->s.origin, MULTICAST_PVS);
	}
}

qboolean EMP_ValidTarget (edict_t *self, edict_t *other)
{
	if (!other || !other->inuse)
		return false;

	// excluded entities (they have no electronics to fry)
	if (other->mtype == M_SKULL || other->mtype == M_MUTANT || other->mtype == M_SUPPLYSTATION 
		|| other->mtype == CTF_PLAYERSPAWN || other->mtype == TOTEM_WATER || other->mtype == TOTEM_AIR 
		|| other->mtype == TOTEM_DARKNESS || other->mtype == TOTEM_NATURE || other->mtype == TOTEM_FIRE 
		|| other->mtype == TOTEM_NATURE)
		return false;

	// can target owner
	if (self->owner && self->owner->inuse && (other == self->owner) && !(other->flags & FL_GODMODE) 
		&& !(other->flags & FL_CHATPROTECT) && visible(self, other))
		return true;

	return G_ValidTarget(self, other, true);
}

qboolean IsAmmoBox (edict_t *ent)
{
	return (ent && ent->inuse && (!strcmp(ent->classname, "ammo_rockets") || !strcmp(ent->classname, "ammo_grenades") 
		|| !strcmp(ent->classname, "ammo_cells") || !strcmp(ent->classname, "ammo_slugs") 
		|| !strcmp(ent->classname, "ammo_bullets") ||!strcmp(ent->classname, "ammo_shells")));
}

void EMP_Explode (edict_t *self)
{
	int		damage;
	float		radius;
	float		time = EMP_INITIAL_TIME + (EMP_ADDON_TIME * self->monsterinfo.level);
	edict_t		*e=NULL;
	qboolean	ammoBox;

	// remove grenade if owner dies or becomes invalid
	if (!G_EntIsAlive(self->owner))
	{
		G_FreeEdict(self);
		return;
	}

	//FIXME: blow up player backpacks too!
	while ((e = findradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		ammoBox = IsAmmoBox(e);

		if (!EMP_ValidTarget(self, e) && !ammoBox)
			continue;

		// blow-up player ammo
		if (e->client)
			ExplodeAmmo(self, e);
		// blow up ammo boxes
		else if (ammoBox)
		{
			damage = EMP_AMMOBOX_INITIAL_DAMAGE+EMP_AMMOBOX_ADDON_DAMAGE*self->monsterinfo.level;
			radius = 0.5*damage;

			if (radius < EMP_MIN_RADIUS)
				radius = EMP_MIN_RADIUS;
			if (radius > EMP_MAX_RADIUS)
				radius = EMP_MAX_RADIUS;

			T_RadiusDamage(self, self->owner, damage, NULL, radius, MOD_EMP);

			// explosion effect
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (e->s.origin);
			gi.multicast (e->s.origin, MULTICAST_PVS);

			// don't respawn if it's a dropped item
			if (!(e->spawnflags & (DROPPED_ITEM|DROPPED_PLAYER_ITEM)))
				SetRespawn(e, 30);
			else
				G_FreeEdict(e);
		}
		// force monsters into idle mode
		else if (!strcmp(e->classname, "drone"))
		{
			// bosses can't be stunned easily
			if (e->monsterinfo.control_cost >= 100 || e->monsterinfo.bonus_flags & BF_UNIQUE_FIRE 
				|| e->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING)
				time *= 0.2;

			e->empeffect_time = level.time + time;
			e->monsterinfo.pausetime = level.time + time;
			e->monsterinfo.stand(e);
		}
		// stun anything else
		else
		{
			e->empeffect_time = level.time + time;
			e->holdtime = level.time + time;
		}
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}

void EMP_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// disappear when touching a sky brush
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// bounce off things that can't be hurt
	if (!other->takedamage)
	{
		if (random() > 0.5)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
	}
	// detonate if we touch a live entity that isn't on our team
	else if (G_EntIsAlive(other) && !OnSameTeam(ent, other))
		EMP_Explode(ent);
	
	// FIXME: change pitch when grenade comes to a stop so the model doesn't poke through the floor
	// this would require changing SV_Physics_Toss() to call touch() function after stopping the entity
}

void fire_emp_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int slevel, float radius, int speed, float timer)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, right, up);

	// spawn grenade entity
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_TRIGGER;
	grenade->s.effects |= EF_GRENADE;
	grenade->s.modelindex = gi.modelindex ("models/objects/ggrenade/tris.md2");
	grenade->owner = self;
	grenade->touch = EMP_Touch;
	grenade->think = EMP_Explode;
	grenade->monsterinfo.level = slevel;
	grenade->dmg_radius = radius;
	grenade->classname = "emp grenade";
	gi.linkentity (grenade);
	grenade->nextthink = level.time + timer;

	// adjust velocity
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
}

void Cmd_TossEMP (edict_t *ent)
{
	int		cost, slevel;
	float	radius;
	vec3_t	forward, right, start, offset;

	//Talent: Bombardier - reduces grenade cost
	cost = EMP_COST - getTalentLevel(ent, TALENT_BOMBARDIER);

	if (!V_CanUseAbilities(ent, EMP, cost, true))
		return;

	slevel = ent->myskills.abilities[EMP].current_level;
	radius = EMP_INITIAL_RADIUS + EMP_ADDON_RADIUS * slevel;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_emp_grenade(ent, start, forward, slevel, radius, 600, 2.0);

	ent->client->ability_delay = level.time + EMP_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;
}

void mirv_explode (edict_t *self)
{
	vec3_t  grenade1, grenade2, grenade3, grenade4;
	vec3_t  grenade5, grenade6, grenade7, grenade8;
	edict_t *e;

	// remove grenade if owner dies or becomes invalid
	if (!G_EntIsAlive(self->owner))
	{
		G_FreeEdict(self);
		return;
	}

	// explosion effect
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
	{
		if (self->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (self->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (self->s.origin);// raise this a bit?
	gi.multicast (self->s.origin, MULTICAST_PVS);

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_MIRV);

	// grenade may have been removed if owner was killed
	if (!self || !self->inuse || !self->owner || !self->owner->inuse)
		return;

	VectorSet(grenade1, 15, 15, 30);
	VectorSet(grenade2, 15, -15, 30);
	VectorSet(grenade3, -15, 15, 30);
	VectorSet(grenade4, -15, -15, 30);

	VectorSet(grenade5, 20, 20, 30);
	VectorSet(grenade6, 20, -20, 30);
	VectorSet(grenade7, -20, 20, 30);
	VectorSet(grenade8, -20, -20, 30);

	// raise starting point?
	e = fire_grenade2(self->owner, self->s.origin, grenade1, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade2, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade2, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade3, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade4, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade5, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade6, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	/*e = fire_grenade2(self->owner, self->s.origin, grenade7, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;
	e = fire_grenade2(self->owner, self->s.origin, grenade8, self->dmg, 10, 1.5, self->dmg_radius, self->radius_dmg, false);
	e->mtype = M_MIRV;
	e->touch = NULL;*/

	G_FreeEdict(self);
}

void mirv_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// disappear when touching a sky brush
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// bounce off things that can't be hurt
	if (!other->takedamage)
	{
		if (random() > 0.5)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
	}
	// detonate if we touch a live entity that isn't on our team
	else if (G_EntIsAlive(other) && !OnSameTeam(ent, other))
		mirv_explode(ent);
}

void fire_mirv_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, float radius, int speed, float timer)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, right, up);

	// spawn grenade entity
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_TRIGGER;
	grenade->s.effects |= EF_GRENADE;
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = mirv_touch;
	grenade->think = mirv_explode;
	grenade->dmg = damage;
	grenade->radius_dmg = damage;
	grenade->dmg_radius = radius;
	grenade->classname = "mirv grenade";
	gi.linkentity (grenade);
	grenade->nextthink = level.time + timer;

	// adjust velocity
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
}

void Cmd_TossMirv (edict_t *ent)
{
	int		damage, cost;
	float	radius;
	vec3_t	forward, right, start, offset;

	//Talent: Bombardier - reduces grenade cost
	cost = MIRV_COST - getTalentLevel(ent, TALENT_BOMBARDIER);

	if (!V_CanUseAbilities(ent, MIRV, MIRV_COST, true))
		return;

	damage = MIRV_INITIAL_DAMAGE + MIRV_ADDON_DAMAGE * ent->myskills.abilities[MIRV].current_level;
	radius = MIRV_INITIAL_RADIUS + MIRV_ADDON_RADIUS * ent->myskills.abilities[MIRV].current_level;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_mirv_grenade(ent, start, forward, damage, radius, 600, 2.0);

	ent->client->ability_delay = level.time + MIRV_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;
}

void organ_remove (edict_t *self, qboolean refund);

void spikeball_findtarget (edict_t *self)
{
	edict_t *e=NULL;

	if (!G_ValidTarget(self, self->enemy, true))
	{
		while ((e = findclosestradius(e, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!G_ValidTarget(self, e, true))
				continue;

			// ignore enemies that are too far from our owner
			// FIXME: we may want to add a hurt function so that we can retaliate if we are attacked out of range
			if (entdist(self->activator, e) > SPIKEBALL_MAX_DIST)
				continue;

			self->enemy = e;
		//	gi.sound (self, CHAN_VOICE, gi.soundindex("misc/scream6.wav"), 1, ATTN_NORM, 0);
			return;
		}

		self->enemy = NULL;
	}
}

void spikeball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other && other->inuse && other->takedamage && !OnSameTeam(ent, other) && (level.time > ent->monsterinfo.attack_finished))
	{
		T_Damage(other, ent, ent, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_UNKNOWN);
		ent->monsterinfo.attack_finished = level.time + 0.5;
	}

	V_Touch(ent, other, plane, surf);
}

qboolean spikeball_recall (edict_t *self)
{
	// we shouldn't be here if recall is disabled, we have a target, our activator is visible
	// or we have teleported recently
	if (/*!self->activator->spikeball_recall || self->enemy ||*/ visible(self, self->activator)
		|| self->monsterinfo.teleport_delay > level.time)
		return false;

	TeleportNearArea(self, self->activator->s.origin, 128, false);
	self->monsterinfo.teleport_delay = level.time + 1;//0.0;
	return true;
}

void spikeball_move (edict_t *self)
{
	vec3_t	start, forward, end, goalpos;
	trace_t	tr;
	float	max_velocity = 350;

	if (self->monsterinfo.attack_finished > level.time)
		return;

	// are we following the player's crosshairs?
	if (self->activator->spikeball_follow)
	{
		G_GetSpawnLocation(self->activator, 8192, NULL, NULL, goalpos);
	}
	// we have no enemy
	else if (!self->enemy)
	{
		if (entdist(self, self->activator) > 128)
		{
			// move towards player
			if (visible(self, self->activator))
			{
				VectorCopy(self->activator->s.origin, goalpos);
				self->monsterinfo.search_frames = 0;

			}
			else
			{
				// try to teleport near player after 3 seconds of losing sight
				if (self->monsterinfo.search_frames > 30)
					spikeball_recall(self);
				else
					self->monsterinfo.search_frames++;
				return;
			}
		}
		else
			return;
	}

	// use the enemy's position as a starting point if we are not following
	if (self->enemy && !self->spikeball_follow)
	{
		// enemy is too far from activator, abort if activator is visible or we can teleport
		if (entdist(self->activator, self->enemy) > SPIKEBALL_MAX_DIST 
			&& (visible(self, self->activator) || spikeball_recall(self)))
		{
			self->enemy = NULL;
			return;
		}

		VectorCopy(self->enemy->s.origin, goalpos);
	}


	// vector to target
	VectorSubtract(goalpos, self->s.origin, forward);
	VectorNormalize(forward);

	// accelerate towards target
	VectorMA(self->velocity, self->monsterinfo.lefty, forward, forward);
	self->velocity[0] = forward[0];
	self->velocity[1] = forward[1];

	// calculate starting position and ending position on a 2-D plane
	VectorCopy(self->s.origin, start);
	VectorAdd(self->s.origin, self->velocity, end);
	tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SHOT);
	VectorCopy(tr.endpos, end);
	start[2] = end[2] = goalpos[2] = 0;
	
	// if we are moving away from our target, slow down
	if (distance(goalpos, end) > distance(goalpos, start))
	{
		self->velocity[0] *= 0.8;
		self->velocity[1] *= 0.8;
	}

	// cap maximum x and y velocity
	if (self->velocity[0] > max_velocity)
		self->velocity[0] = max_velocity;
	if (self->velocity[0] < -max_velocity)
		self->velocity[0] = -max_velocity;
	if (self->velocity[1] > max_velocity)
		self->velocity[1] = max_velocity;
	if (self->velocity[1] < -max_velocity)
		self->velocity[1] = -max_velocity;

//	gi.dprintf("%.0f %.0f %.0f\n", self->velocity[0], self->velocity[1], self->velocity[2]);
}

void spikeball_idle_friction (edict_t *self)
{
	if (!self->enemy && !self->spikeball_follow)
	{
		self->velocity[0] *= 0.8;
		self->velocity[1] *= 0.8;

		if (VectorLength(self->velocity) < 10)
		{
			VectorClear(self->velocity);
			return;
		}
		
		if (level.time > self->msg_time)
		{
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("spore/neutral%d.wav", GetRandom(1, 6))), 1, ATTN_NORM, 0);
			self->msg_time = level.time + GetRandom(2, 10);
		}
	}
}

void spikeball_effects(edict_t *self)
{
	if (self->enemy)
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_RED;
	}
	else
	{
		self->s.effects &= ~EF_COLOR_SHELL;
		self->s.renderfx &= ~RF_SHELL_RED;
	}
}

void spikeball_think (edict_t *self)
{
	if (!G_EntIsAlive(self->owner) || level.time > self->delay)
	{
		organ_remove(self, false);
		return;
	}
	else if (self->removetime > 0)
	{
		qboolean converted=false;

		if (self->flags & FL_CONVERTED)
			converted = true;

		if (level.time > self->removetime)
		{
			// if we were converted, try to convert back to previous owner
			if (converted && self->prev_owner && self->prev_owner->inuse)
			{
				if (!ConvertOwner(self->prev_owner, self, 0, false))
				{
					organ_remove(self, false);
					return;
				}
			}
			else
			{
				organ_remove(self, false);
				return;
			}
		}
		// warn the converted monster's current owner
		else if (converted && self->activator && self->activator->inuse && self->activator->client 
			&& (level.time > self->removetime-5) && !(level.framenum%10))
				safe_cprintf(self->activator, PRINT_HIGH, "%s conversion will expire in %.0f seconds\n", 
					V_GetMonsterName(self), self->removetime-level.time);	
	}

	if (!M_Upkeep(self, 13, 1))
		return;

	spikeball_effects(self);
	spikeball_findtarget(self);
	//spikeball_recall(self);
	spikeball_move(self);
	spikeball_idle_friction(self);

	self->nextthink = level.time + FRAMETIME;
}

void spikeball_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (!self->monsterinfo.slots_freed && self->activator && self->activator->inuse)
	{
		int cur;

		self->activator->num_spikeball--;
		cur = self->activator->num_spikeball;
		self->monsterinfo.slots_freed = true;
		
		if (PM_MonsterHasPilot(attacker))
			attacker = attacker->owner;

		if (attacker->client)
			safe_cprintf(self->activator, PRINT_HIGH, "Your spore was killed by %s (%d remain)\n", attacker->client->pers.netname, cur);
		else if (attacker->mtype)
			safe_cprintf(self->activator, PRINT_HIGH, "Your spore was killed by a %s (%d remain)\n", V_GetMonsterName(attacker), cur);
		else
			safe_cprintf(self->activator, PRINT_HIGH, "Your spore was killed by a %s (%d remain)\n", attacker->classname, cur);
	}

	organ_remove(self, false);
}

void fire_spikeball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int throw_speed, int ball_speed, float radius, int duration, int health, int skill_level)
{
	int		cost = SPIKEBALL_COST;
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((self->mtype == MORPH_FLYER && self->myskills.abilities[FLYER].current_level < 5) 
		|| (self->mtype == MORPH_CACODEMON && self->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, right, up);

	// spawn grenade entity
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	grenade->takedamage = DAMAGE_AIM;
	grenade->die = spikeball_die;
	grenade->health = grenade->max_health = health;
	grenade->movetype = MOVETYPE_BOUNCE;
//	grenade->svflags |= SVF_MONSTER; // tell physics to clip on more than just walls
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GIB;
	grenade->s.modelindex = gi.modelindex ("models/objects/sspore/tris.md2");
	grenade->owner = grenade->activator = self;
	grenade->touch = spikeball_touch;
	grenade->think = spikeball_think;
	grenade->dmg = damage;
	grenade->mtype = M_SPIKEBALL;
	grenade->monsterinfo.level = skill_level;
	grenade->dmg_radius = radius;
	grenade->monsterinfo.attack_finished = level.time + 2.0;
	grenade->monsterinfo.lefty = ball_speed;
	grenade->delay = level.time + duration;
	grenade->classname = "spikeball";
	VectorSet(grenade->maxs, 8, 8, 8);
	VectorSet(grenade->mins, -8, -8, -8);
	gi.linkentity (grenade);
	grenade->nextthink = level.time + 1.0;
	grenade->monsterinfo.cost = cost;

	// adjust velocity
	VectorScale (aimdir, throw_speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);

	self->num_spikeball++;
}

void organ_removeall (edict_t *ent, char *classname, qboolean refund);

void Cmd_TossSpikeball (edict_t *ent)
{
	int		talentLevel;
	int		cost = SPIKEBALL_COST, max_count = SPIKEBALL_MAX_COUNT;
	int		health = SPIKEBALL_INITIAL_HEALTH + SPIKEBALL_ADDON_HEALTH * ent->myskills.abilities[SPORE].current_level;
	int		damage = SPIKEBALL_INITIAL_DAMAGE + SPIKEBALL_ADDON_DAMAGE * ent->myskills.abilities[SPORE].current_level;
	float	duration = SPIKEBALL_INITIAL_DURATION + SPIKEBALL_ADDON_DURATION * ent->myskills.abilities[SPORE].current_level;
	float	range = SPIKEBALL_INITIAL_RANGE + SPIKEBALL_ADDON_RANGE * ent->myskills.abilities[SPORE].current_level;
	vec3_t	forward, right, start, offset;
	
	if (ent->num_spikeball > 0 && Q_strcasecmp (gi.args(), "move") == 0)
	{
		// toggle movement setting
		if (ent->spikeball_follow)
		{
			ent->spikeball_follow = false;
			safe_cprintf(ent, PRINT_HIGH, "Spores will follow targets\n");
		}
		else
		{
			ent->spikeball_follow = true;
			safe_cprintf(ent, PRINT_HIGH, "Spores will follow your crosshairs\n");
		}
		return;
	}
/*
	if (ent->num_spikeball > 0 && Q_strcasecmp (gi.args(), "recall") == 0)
	{
		// toggle recall setting
		if (ent->spikeball_recall)
		{
			ent->spikeball_recall = false;
			safe_cprintf(ent, PRINT_HIGH, "Spores recall disabled\n");
		}
		else
		{
			ent->spikeball_recall = true;
			safe_cprintf(ent, PRINT_HIGH, "Spores recall enabled\n");
		}
		return;
	}
*/
	if (Q_strcasecmp (gi.args(), "help") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "syntax: spore [move|remove]\n");
		return;
	}

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		organ_removeall(ent, "spikeball", true);
		safe_cprintf(ent, PRINT_HIGH, "Spores removed\n");
		ent->num_spikeball = 0;
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	if (!V_CanUseAbilities(ent, SPORE, cost, true))
		return;

	//Talent: Swarming
	if ((talentLevel = getTalentLevel(ent, TALENT_SWARMING)) > 0)
	{
		// max_count *= 1.0 + 0.2 * talentLevel;
		damage *= 1.0 + 0.15 * talentLevel; // Only increase damage.
	}

	if (ent->num_spikeball >= max_count)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the maximum amount of spores (%d)\n", ent->num_spikeball);
		return;
	}

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_spikeball(ent, start, forward, damage, 600, 100, range, duration, health, ent->myskills.abilities[SPORE].current_level);

	ent->client->ability_delay = level.time + SPIKEBALL_DELAY;

	ent->client->pers.inventory[power_cube_index] -= cost;
}

void acid_sparks (vec3_t org, int num, float radius)
{
	int		i;
	vec3_t	start;

	// 0 = black, 8 = grey, 15 = white, 16 = light brown, 20 = brown, 57 = light orange, 66 = orange/red, 73 = maroon
	// 106 = pink, 113 = light blue, 119 = blue, 123 = dark blue, 200 = pale green, 205 = dark green, 209 = bright green
	// 217 = white, 220 = yellow, 226 = orange, 231 = red/orange, 240 = red, 243 = dark blue

	for (i=0; i<num; i++)
	{
		//VectorCopy(self->s.origin, start);
		VectorCopy(org, start);
		start[0] += crandom() * GetRandom(0, (int) radius);
		start[1] += crandom() * GetRandom(0, (int) radius);
		//start[2] += crandom() * GetRandom(0, (int) self->dmg_radius);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_LASER_SPARKS);
		gi.WriteByte(1); // number of sparks
		gi.WritePosition(start);
		gi.WriteDir(vec3_origin);
		//gi.WriteByte(GetRandom(200, 209)); // color
		if (random() <= 0.33)
			gi.WriteByte(205);
		else
			gi.WriteByte(209);
		gi.multicast(start, MULTICAST_PVS);
	}
}

void CreatePoison (edict_t *ent, edict_t *targ, int damage, float duration, int meansOfDeath);

void acid_explode (edict_t *self)
{
	int amt, max_amt;
	vec3_t start;
	edict_t *e=NULL;

	if (!G_EntIsAlive(self->owner))
	{
		G_FreeEdict(self);
		return;
	}

	while ((e = findradius(e, self->s.origin, self->dmg_radius)) != NULL)
	{
		// acid heals alien summons beyond their normal max health
		if (OnSameTeam(self->owner, e) && (e->mtype == M_SPIKER
			|| e->mtype == M_OBSTACLE || e->mtype == M_GASSER
			|| e->mtype == M_HEALER || e->mtype == M_COCOON))
		{
			amt = 0.1 * self->dmg;
			max_amt = 2 * e->max_health;

			// 2x health maximum
			if (e->health + amt < max_amt)
				e->health += amt;
			else if (e->health < max_amt)
				e->health = max_amt;
		}

		if (e && e->inuse && e->takedamage && e->solid != SOLID_NOT && !OnSameTeam(self->owner, e))
		{
			T_Damage(e, self, self->owner, vec3_origin, e->s.origin, vec3_origin, self->dmg, 0, 0, MOD_ACID);
			CreatePoison(self->owner, e, self->radius_dmg, self->delay, MOD_ACID);
		}
	}

	VectorCopy(self->s.origin, start);
	start[2] += 8;
	acid_sparks(start, 20, self->dmg_radius);

	gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/acid.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict(self);
}


void acid_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// disappear when touching a sky brush
	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	acid_explode(ent);
}

void fire_acid (edict_t *self, vec3_t start, vec3_t aimdir, int projectile_damage, float radius, 
				int speed, int acid_damage, float acid_duration)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	// calling entity made a sound, used to alert monsters
	self->lastsound = level.framenum;

	// get aiming angles
	vectoangles(aimdir, dir);
	// get directional vectors
	AngleVectors(dir, forward, right, up);

	// spawn grenade entity
	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	grenade->movetype = MOVETYPE_TOSS;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.modelindex = gi.modelindex ("models/objects/spore/tris.md2");
	grenade->s.skinnum = 1;
	grenade->owner = self;
	grenade->touch = acid_touch;
	grenade->think = acid_explode;
	grenade->dmg = projectile_damage;
	grenade->radius_dmg = acid_damage;
	grenade->dmg_radius = radius;
	grenade->delay = acid_duration;
	grenade->classname = "acid";
	gi.linkentity (grenade);
	grenade->nextthink = level.time + 10.0;

	// adjust velocity
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
}

#define ACID_INITIAL_DAMAGE		50
#define ACID_ADDON_DAMAGE		15
#define ACID_INITIAL_SPEED		600
#define ACID_ADDON_SPEED		0
#define ACID_INITIAL_RADIUS		64
#define ACID_ADDON_RADIUS		0
#define ACID_DURATION			10.0
#define ACID_DELAY				0.2
#define ACID_COST				20

void Cmd_FireAcid_f (edict_t *ent)
{
	int		damage = ACID_INITIAL_DAMAGE + ACID_ADDON_DAMAGE * ent->myskills.abilities[ACID].current_level;
	int		speed = ACID_INITIAL_SPEED + ACID_ADDON_SPEED * ent->myskills.abilities[ACID].current_level;
	float	radius = ACID_INITIAL_RADIUS + ACID_ADDON_RADIUS * ent->myskills.abilities[ACID].current_level;
	vec3_t	forward, right, start, offset;

	if (!V_CanUseAbilities(ent, ACID, ACID_COST, true))
		return;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	fire_acid(ent, start, forward, damage, radius, speed, (int)(0.1 * damage), ACID_DURATION);

	ent->client->ability_delay = level.time + ACID_DELAY;
	ent->client->pers.inventory[power_cube_index] -= ACID_COST;
}
