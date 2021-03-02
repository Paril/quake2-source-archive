/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
vec3_t hb_dir[HB_DIR_NUM] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}, {0, 0, -1},
									{1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {-1, 1, 1}, {-1, 0, 1}, {-1, -1, 1}, {0, -1, 1}, {1, -1, 1},
									{1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {-1, 1, 0}, {-1, 0, 0}, {-1, -1, 0}, {0, -1, 0}, {1, -1, 0},
									{1, 0, -1}, {1, 1, -1}, {0, 1, -1}, {-1, 1, -1}, {-1, 0, -1}, {-1, -1, -1}, {0, -1, -1}, {1, -1, -1} };

//vec3_t check_dodge_dir[DODGE_DIR_NUM] = { { 1, 1, 0 }, { 1, 0, 0 }, { -1, -1, 0 }, { 0, 1, 0 }, { 0, 0, 0 }, { 0, -1, 0 }, { 0, 0, 0 }, { 0, 1, 0 }, { 0, 1, 0 } };

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/

void silencer_degrade(edict_t *self, int damage)
{
	if (!self->client)
		return;

	if (item_mod)
	{
		self->client->pers.sile_health -= damage;
		if (self->client->pers.sile_health < 0)
			self->client->pers.sile_health = 0;
	}
}
void check_dodge(edict_t *self, vec3_t start, vec3_t dir_org, int speed, int priority)
{
	vec3_t	end;
	vec3_t	v, dir;
	trace_t	tr;
	float	eta;
	int checks = 0;

	// easy mode only ducks one quarter the time
	/*if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}*/
check:
	
	VectorCopy(dir_org, dir);
	if (checks)
	{
		dir[0] += crandom() * 0.33;
		dir[1] += crandom() * 0.33;
		dir[2] += crandom() * 0.33;
	}
	VectorMA(start, 8192, dir, end);

	
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract(tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		if (!priority)
		{
			if ((strcmp(tr.ent->classname, "SP_monster_soldier_light") == 0 || strcmp(tr.ent->classname, "SP_monster_soldier") == 0 || strcmp(tr.ent->classname, "SP_monster_soldier_ss") == 0) && skill->value > 2)
			{
				if (random() > 0.25)
					return;
			}
			else
			{
				if (random() > 0.1)
					return;
			}

		}


		tr.ent->monsterinfo.dodge(tr.ent, self, eta);
		return;
	}
	checks++;
	//debug_trail(start, end);
	//if(strcmp(tr.ent->classname, "worldspawn"))
	//	tr.ent->velocity[2] += 500;
	if (checks < DODGE_CHECK_NUM)
		goto check;
}


/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit(edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	//see if enemy is in range
	if (self->movetype != MOVETYPE_WALK)
	{
		VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
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

	}

	VectorMA(self->s.origin, range, dir, point);

	VectorScale(aim, 2.0, aim);
	tr = gi.trace(self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}
	if (self->movetype == MOVETYPE_WALK)
		AngleVectors(self->client->v_angle, forward, right, up);
	else
		AngleVectors(self->s.angles, forward, right, up);
	VectorMA(self->s.origin, range, forward, point);
	VectorMA(point, aim[1], right, point);
	VectorMA(point, aim[2], up, point);
	if (self->movetype != MOVETYPE_WALK)
		VectorSubtract(point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage(tr.ent, self, self, dir, point, vec3_origin, damage, kick / 2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	if (self->movetype != MOVETYPE_WALK)
		VectorMA(self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract(v, point, v);
	VectorNormalize(v);
	if (self->movetype != MOVETYPE_WALK)
	{
		VectorMA(self->enemy->velocity, kick, v, self->enemy->velocity);
		if (self->enemy->velocity[2] > 0)
			self->enemy->groundentity = NULL;
	}
	return true;
}

void kick_hit(edict_t *self, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up, angles, start, offset;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;
	if (deathmatch->value)
		damage *= 0.66;
	if (self->client)
		check_dodge(self, start, dir, 999, DODGE_PRIORITY_LOW);
	VectorAdd(self->client->v_angle, self->client->kick_angles, angles);
	AngleVectors(angles, forward, right, NULL);
	VectorSet(offset, -8, 8, self->viewheight - 16);
	P_ProjectSource(self->client, self->s.origin, offset, forward, right, start);
	VectorMA(start, KICK_LEG_DISTANCE, forward, forward);

	tr = gi.trace(start, NULL, NULL, forward, self, MASK_SHOT);

	/*if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return;
	}*/

	if (tr.fraction < 1 && self->client->kick == KICK_KICK)
	{
		self->client->kick = KICK_KICK2;
		gi.positioned_sound(tr.endpos, tr.ent, CHAN_BODY, gi.soundindex("infantry/melee2.wav"), 1, ATTN_IDLE, 0);
	}
	//gi.WriteByte(svc_temp_entity);
	//gi.WriteByte(TE_GUNSHOT);
	//gi.WritePosition(start);
	///gi.WriteDir(tr.plane.normal);
	//gi.multicast(start, MULTICAST_PVS);
	//gi.WriteByte(svc_temp_entity);
	//gi.WriteByte(TE_GUNSHOT);
	//gi.WritePosition(tr.endpos);
	//gi.WriteDir(tr.plane.normal);
	//gi.multicast(tr.endpos, MULTICAST_PVS);


	//gi.WriteByte(svc_temp_entity);
	//gi.WriteByte(TE_GUNSHOT);
	//gi.WritePosition(forward);
	//gi.WriteDir(tr.plane.normal);
	//gi.multicast(forward, MULTICAST_PVS);
	// do the damage
	damage = damage + (damage * (VectorLength(self->velocity) / 300));

	if (tr.ent->mass < 300 && !(tr.ent->spawnflags & DEAD_DEAD) && tr.ent->movetype == MOVETYPE_STEP)
		tr.ent->velocity[2] += 100 + damage;

	
	T_Damage(tr.ent, self, self, forward, self->s.origin, tr.plane.normal, damage, damage * 4, 0 , MOD_HIT);
	

	// do our special form of knockback here
	if (self->movetype != MOVETYPE_WALK)
		VectorMA(tr.ent->absmin, 0.5, tr.ent->size, v);
	VectorSubtract(v, tr.endpos, v);
	VectorNormalize(v);
	return;
	if (self->movetype != MOVETYPE_WALK)
	{
		VectorMA(self->enemy->velocity, kick, v, tr.ent->velocity);
		if (self->enemy->velocity[2] > 0)
			self->enemy->groundentity = NULL;
	}
	return;
}
/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
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

	silencer_degrade(self, damage);
	

	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles(aimdir, dir);
		AngleVectors(dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA(start, 8192, forward, end);
		VectorMA(end, r, right, end);
		VectorMA(end, u, up, end);

		if (gi.pointcontents(start) & MASK_WATER)
		{
			water = true;
			VectorCopy(start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace(start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy(tr.endpos, water_start);

			if (!VectorCompare(start, tr.endpos))
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
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(TE_SPLASH);
					gi.WriteByte(50 + rand() % 50);
					gi.WritePosition(tr.endpos);
					gi.WriteDir(tr.plane.normal);
					gi.WriteByte(color);
					gi.multicast(tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract(end, start, dir);
				vectoangles(dir, dir);
				AngleVectors(dir, forward, right, up);
				r = crandom()*hspread * 10;
				u = crandom()*vspread * 10;
				VectorMA(water_start, 8192, forward, end);
				VectorMA(end, r, right, end);
				VectorMA(end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace(water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp(tr.surface->name, "sky", 3) != 0)
				{
					if (te_impact == TE_SHOTGUN)
					{
						if ((mod == MOD_SHOTGUN || mod == MOD_SSHOTGUN || mod == MOD_RAILGUN_FRAG) && rand() % 100 < 25)
							te_impact = TE_GUNSHOT;
					}
				
						gi.WriteByte(svc_temp_entity);
						gi.WriteByte(te_impact);
						gi.WritePosition(tr.endpos);
						gi.WriteDir(tr.plane.normal);
						gi.multicast(tr.endpos, MULTICAST_PVS);

					
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

		VectorSubtract(tr.endpos, water_start, dir);
		VectorNormalize(dir);
		VectorMA(tr.endpos, -2, dir, pos);
		if (gi.pointcontents(pos) & MASK_WATER)
			VectorCopy(pos, tr.endpos);
		else
			tr = gi.trace(pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd(water_start, tr.endpos, pos);
		VectorScale(pos, 0.5, pos);

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BUBBLETRAIL);
		gi.WritePosition(water_start);
		gi.WritePosition(tr.endpos);
		gi.multicast(pos, MULTICAST_PVS);
	}
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	if (self->client)
		check_dodge(self, start, aimdir, 9999, DODGE_PRIORITY_LOW);
	fire_lead(self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	/*if (self->client)
	{
		vec3_t dir;
		VectorAdd(self->client->ps.gunoffset, start, dir);
		VectorMA(dir, 20, aimdir, dir);
		dir[2] += 4;
		spawn_m_muzzleflash(self, dir, aimdir, 0);
	}*/

	int		i;
	if (self->client)
		check_dodge(self, start, aimdir, 9999, DODGE_PRIORITY_HIGH);

	for (i = 0; i < count; i++)
		fire_lead(self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/

void blaster_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	vec3_t normalc;
	if (!plane->normal)
		VectorCopy(self->s.origin, normalc);
	else
		VectorCopy(plane->normal, normalc);
	
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}
	
	//gi.bprintf(PRINT_HIGH, "NORMAL = NULL !!!!");
	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_HYPERBLASTER;
		else
			mod = MOD_BLASTER;
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, normalc, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else
	{
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BLASTER);
		gi.WritePosition(self->s.origin);
		if (!plane)
			gi.WriteDir(vec3_origin);
		else
			gi.WriteDir(plane->normal);
		gi.multicast(self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict(self);
}
static void Grenade_Explode(edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	if (ent->spawnflags == 6)
	{
		ent->nextthink = level.time + 0.1;
		ent->spawnflags = 24;
		return;

	}
	else if (ent->spawnflags == 24)
	{
		ent->nextthink = level.time + 0.1;
		ent->velocity[2] += 600;
		ent->spawnflags = 12;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/GRENLF1A.WAV"), 1, ATTN_IDLE, 0);
		return;
	}
	else if (ent->spawnflags == 12)
	{
		vec3_t forward, right, up;
		VectorClear(ent->s.angles);
		ent->s.angles[YAW] = anglemod(crandom() * 1000);
		AngleVectors(ent->s.angles, forward, right, up);

		fire_grenade(ent->owner, ent->s.origin, forward, ent->dmg, 600, 0.2, ent->dmg_radius);

		VectorScale(forward, -1, forward);

		fire_grenade(ent->owner, ent->s.origin, forward, ent->dmg, 600, 0.2, ent->dmg_radius);

		fire_grenade(ent->owner, ent->s.origin, right, ent->dmg, 600, 0.2, ent->dmg_radius);

		VectorScale(forward, -1, forward);

		fire_grenade(ent->owner, ent->s.origin, right, ent->dmg, 600, 0.1, ent->dmg_radius);

		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/GRENLF1A.WAV"), 1, ATTN_IDLE, 0);

		G_FreeEdict(ent);
		return;
	}
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd(ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA(ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract(ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength(v);
		VectorSubtract(ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage(ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA(ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte(svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte(TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte(TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte(TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition(origin);
	gi.multicast(ent->s.origin, MULTICAST_PHS);
	//gi.bprintf(PRINT_HIGH, "SHOTGUN GRENADE TOUCH: origin = %s\n", vtos(ent->s.origin));

	G_FreeEdict(ent);
}

void shotgun_grenade_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	//gi.bprintf(PRINT_HIGH, "SHOTGUN GRENADE TOUCH: origin = %s time = %f\n", vtos(self->s.origin), level.time);

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		//gi.bprintf(PRINT_HIGH, "SHOTGUN GRENADE TOUCH DAMAGE\n");
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, 5, 1, DAMAGE_NO, MOD_SHOTGUN);
	}

	if (random() > 0.5)
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"), 1, ATTN_IDLE, 0);
	else
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"), 1, ATTN_IDLE, 0);
	self->enemy = other;
	Grenade_Explode(self);

	G_FreeEdict(self);
}
void blaster_think(edict_t *ent)
{
	if (ent->style)
	{
		ent->s.modelindex = gi.modelindex("models/objects/laser/tris_slow.md2");
		//ent->s.modelindex = gi.modelindex("models/objects/grenade_h/tris.md2");
		ent->style = 0;
	}

	ent->nextthink = level.time + 0.1;
	vectoangles(ent->velocity, ent->s.angles); //update angle

	if (gi.pointcontents(ent->s.origin) & MASK_WATER)
		VectorScale(ent->velocity, 0.99, ent->velocity);


	VectorScale(ent->velocity, 1 - 0.01, ent->velocity);
	//ent->velocity[2] -= (2500 - VectorLength(ent->velocity)) / 250;
	add_gravity(ent);

	if (ent->delay < level.time)
		G_FreeEdict(ent);
}

void add_gravity(edict_t *ent)
{
	if (ent->waterlevel == 0 && !ent->groundentity)
		ent->velocity[2] -= ent->gravity * sv_gravity->value * FRAMETIME * 0.05 * blaster_drop->value;
}
void fire_shotgun_grenade(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

	silencer_degrade(self, damage);

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy(start, bolt->s.origin);
	VectorCopy(start, bolt->s.old_origin);
	vectoangles(dir, bolt->s.angles);
	bolt->s.modelindex = gi.modelindex("models/objects/grenade_s/tris.md2");



	bolt->s.effects |= EF_GRENADE;
	VectorScale(dir, speed, bolt->velocity);
	if (gi.pointcontents(start) & MASK_WATER)
		VectorScale(bolt->velocity, 0.33, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear(bolt->mins);
	VectorClear(bolt->maxs);

	bolt->owner = self;
	bolt->touch = shotgun_grenade_touch;
	bolt->nextthink = level.time + 0.1;
	bolt->think = blaster_think;
	bolt->delay = 3 + level.time;


	bolt->dmg_radius = damage;
	bolt->dmg = damage;
	bolt->classname = "sgrenade";

	gi.linkentity(bolt);

	if (self->client)
		check_dodge(self, bolt->s.origin, dir, speed, DODGE_PRIORITY_HIGH);

	tr = gi.trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch(bolt, tr.ent, NULL, NULL);
	}

}

void fire_blaster(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, int type)
{
	
	edict_t	*bolt;
	trace_t	tr;
	//gi.bprintf(PRINT_HIGH, "type = %i\n", type);
	
	silencer_degrade(self, damage);

	VectorNormalize(dir);
	if (self->movetype == MOVETYPE_WALK)
	{
		if (self->client->pers.weapon_ext.scounter > BLASTER_MAX_CHARGE)
			self->client->pers.weapon_ext.scounter = BLASTER_MAX_CHARGE;

	}
	
	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	VectorCopy(start, bolt->s.origin);
	VectorCopy(start, bolt->s.old_origin);
	vectoangles(dir, bolt->s.angles);

	bolt->s.modelindex = gi.modelindex("models/objects/laser/tris.md2");
	int charge = 0;
	if (self->movetype == MOVETYPE_WALK && self->client->pers.weapon == FindItem("blaster"))
		charge = self->client->pers.weapon_ext.scounter;
	else if (self->monsterinfo.aiflags & AI_CHARGEDSHOT)
		charge = self->monsterinfo.charge;
	//gi.bprintf(PRINT_HIGH, "BLASTER: charge = %i\n", charge);

	

	if (self->movetype == MOVETYPE_STEP || self->movetype == MOVETYPE_WALK && self->client->pers.weapon == FindItem("blaster"))
	{
		damage = (int)(damage * blaster_charge_calc(self, charge));
	}
	
	if (charge)
	{
		speed = (int)(speed * blaster_charge_calc(self, charge));
		bolt->s.modelindex = gi.modelindex("models/objects/laser/tris_c.md2");
		bolt->s.effects |= EF_GREENGIB;
		bolt->s.effects |= EF_GRENADE;
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/BLASTF2A.WAV"), (0.5 + (charge / (BLASTER_MAX_CHARGE * 2))), ATTN_NORM, 0); //
		if (self->movetype == MOVETYPE_STEP)
		{
			//gi.bprintf(PRINT_HIGH, "BLASTER: charge = %i\n", charge);
			self->s.sound = 0;
			self->monsterinfo.charge = 0;
			self->monsterinfo.aiflags &= ~AI_CHARGEDSHOT;
			gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/BLASTSPSP.wav"), 1, ATTN_IDLE, 0);

		}
	}
	else
		bolt->s.effects |= effect;

	if (type == BOLT_NOISY2_HB_SLOW)
	{
		bolt->style = 1;

	}

	VectorScale(dir, speed, bolt->velocity);
	if (gi.pointcontents(start) == MASK_WATER)
		VectorScale(bolt->velocity, 0.33, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->s.renderfx |= RF_NOSHADOW;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear(bolt->mins);
	VectorClear(bolt->maxs);
	if (type == BOLT_NOISY2_HB || type == BOLT_NOISY2_HB_SLOW)
		bolt->s.sound = gi.soundindex("misc/lasfly.wav");
	else if (type == BOLT_NOISY || type == BOLT_NOISY_HB)
		bolt->s.sound = gi.soundindex("misc/lasfly_q.wav");
	bolt->owner = self;	
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 0.05;
	bolt->think = blaster_think;
	bolt->delay = 3 + level.time;

	

	bolt->dmg = damage;
	bolt->classname = "bolt";
	if (type == BOLT_NOISY_HB || type == BOLT_SILENT_HB)
		bolt->spawnflags = 1;
	gi.linkentity(bolt);

	if (self->client)
		check_dodge(self, bolt->s.origin, dir, speed, DODGE_PRIORITY_LOW);

	tr = gi.trace(self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA(bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch(bolt, tr.ent, NULL, NULL);
	}
	if (self->movetype == MOVETYPE_WALK && self->client->pers.weapon == FindItem("blaster"))
	{
		//gi.bprintf(PRINT_HIGH, "BLASTER_CHARGE_CALC = %f\n", blaster_charge_calc(self));


			//self->client->pers.weapon_ext.scounter = 0;

	}
}


/*
=================
fire_grenade
=================
*/
static void GrenadeGravity_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	
	if (other == ent->owner)
		return;

	if (!surf)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!ent->style)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"), 1, ATTN_IDLE, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"), 1, ATTN_IDLE, 0);
		}
		else
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/grenlb1b.wav"), 1, ATTN_IDLE, 0);
		}
	}
	

	if ((strcmp(other->classname, "func_wall") || strcmp(other->classname, "func_door") || strcmp(other->classname, "func_plat") || strcmp(other->classname, "func_train") || strcmp(other->classname, "worldspawn"))
		&&(strstr(surf->name, "met")  || strstr(surf->name, "grat")  || strstr(surf->name, "dr")  || strstr(surf->name, "door")
		|| strstr(surf->name, "mt")  || strstr(surf->name, "wslt")  || strstr(surf->name, "supprt")  || strstr(surf->name, "wtroof")
		|| strstr(surf->name, "yellow")  || strstr(surf->name, "comp")  || strstr(surf->name, "timpod")  || strstr(surf->name, "station")
		|| strstr(surf->name, "pip")  || strstr(surf->name, "lite")  || strstr(surf->name, "met")  || strstr(surf->name, "ceil")
		|| strstr(surf->name, "elev")  || strstr(surf->name, "box")  || strstr(surf->name, "belt")  || strstr(surf->name, "stripe")
		|| strstr(surf->name, "train")  || strstr(surf->name, "troof")  || strstr(surf->name, "dor")  || strstr(surf->name, "sltfr")
		|| strstr(surf->name, "slot")  || strstr(surf->name, "stair")  || strstr(surf->name, "gle")  || strstr(surf->name, "mach")
		|| strstr(surf->name, "blbk")  || strstr(surf->name, "blum11")  || strstr(surf->name, "broken")  || strstr(surf->name, "core")
		|| strstr(surf->name, "dump")  || strstr(surf->name, "grn")  || strstr(surf->name, "wires")  || strstr(surf->name, "lt")
		|| strstr(surf->name, "p_flr")  || strstr(surf->name, "pow")  || strstr(surf->name, "pthnm")  || strstr(surf->name, "wndw")
		|| strstr(surf->name, "reds")  || strstr(surf->name, "pth")  || strstr(surf->name, "geo")  || strstr(surf->name, "core") ))

	{
		
		vec3_t dir;
		if(plane)
		VectorCopy(plane->normal, dir);
		if (!ent->style)
		{
			//gi.bprintf(PRINT_HIGH, "GRAVITY GRENADE SHOULD STICK!!\n");
			VectorClear(ent->avelocity);
			VectorScale(ent->velocity, 0.5, ent->velocity);
			VectorMA(ent->velocity, 999, dir, ent->velocity);
			ent->style = 1;
		}
		else
			VectorMA(ent->velocity, 500, dir, ent->velocity);






	}

	
}
static void Grenade_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb1a.wav"), 1, ATTN_IDLE, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/hgrenb2a.wav"), 1, ATTN_IDLE, 0);
		}
		else
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/grenlb1b.wav"), 1, ATTN_IDLE, 0);
		}
		if (ent->spawnflags == 6)
		{
			Grenade_Explode(ent);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode(ent);
}
void grenade_think(edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	M_avoid_danger(self);
	if (gi.pointcontents(self->s.origin) & MASK_WATER)
		VectorScale(self->velocity, 0.9, self->velocity);
	if (self->delay < level.time)
	{
		Grenade_Explode(self);
	}

}
void fire_grenade(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);


	silencer_degrade(self, damage);

	grenade = G_Spawn();
	VectorCopy(start, grenade->s.origin);
	VectorScale(aimdir, speed, grenade->velocity);
	VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);

	if (gi.pointcontents(start) & MASK_WATER)
		VectorScale(grenade->velocity, 0.33, grenade->velocity);

	VectorSet(grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear(grenade->mins);
	VectorClear(grenade->maxs);
	grenade->s.modelindex = gi.modelindex("models/objects/grenade_h/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + FRAMETIME;
	grenade->delay = level.time + timer;
	grenade->think = grenade_think;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";
	if (self->movetype == MOVETYPE_WALK && speed == 1201)
	{
		if (speed == 1201)
			grenade->spawnflags = 6;


	}



	if (self->client)
		check_dodge(self, start, dir, speed, DODGE_PRIORITY_HIGH);
	dir[2] -= 64;
	if (self->client)
		check_dodge(self, start, dir, speed, DODGE_PRIORITY_HIGH);
	dir[2] -= 64;
	if (self->client)
		check_dodge(self, start, dir, speed, DODGE_PRIORITY_HIGH);

	gi.linkentity(grenade);
}

void Grenade_Gravity(edict_t *self)
{
	
	self->nextthink = level.time + 0.1;
	
	self->noise_index2 = 100;

	M_avoid_danger(self);

	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;
	vec3_t dir, dir_copy;
	int mod;
	// the BFG effect
	ent = NULL;
	//gi.bprintf(PRINT_HIGH, "fabs(sin(self->noise_index2)) = %f\n", 1 + sin(self->noise_index2));
	int include = 0;
	
	while ((ent = findradius(ent, self->s.origin, self->dmg_radius )) != NULL)
	{
		if (ent == self)
			continue; 
		if (strcmp(ent->classname, "func_wall") == 0 || strcmp(ent->classname, "func_door") == 0 || strcmp(ent->classname, "func_plat") == 0 || strcmp(ent->classname, "func_train") == 0 || strcmp(ent->classname, "func_button") == 0)
			continue;
		if (ent->s.effects & (EF_BFG | EF_GIB | EF_GRENADE | EF_GREENGIB | EF_ROCKET | EF_BLASTER | EF_HYPERBLASTER | EF_ROTATE) || ent->s.renderfx & RF_GLOW ||
			(ent->svflags & SVF_MONSTER) || ent->client || !strcmp(ent->classname, "misc_explobox") || !strcmp(ent->classname, "bodyque"))
			include = 1;
		if (!ent->takedamage && !include)
			continue;
		if (ent->movetype == MOVETYPE_NONE)
			continue; 
		if ( !include)
			continue;

		if (!CanDamage(ent, self))
			continue;

		
		//gi.bprintf(PRINT_HIGH, "AFFECTED = %s\n", ent->classname);
		VectorSubtract(self->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		//VectorMA(ent->velocity, 100 * (1+ sin(self->noise_index2)), dir, dir);
		VectorScale(dir, 400 * fabs((sin(self->noise_index2))), dir);
		dir[0] *= 1 + (crandom() * 0.25);
		dir[1] *= 1 + (crandom() * 0.25);
		dir[2] *= 1 + (crandom() * 0.25);
		VectorAdd(ent->velocity, dir, ent->velocity);
		VectorCopy(dir, dir_copy);
		VectorScale(dir, -0.1, dir);
		dir[2] *= 0.25;
		if (VectorLength(ent->avelocity) < 500)
		{
			ent->avelocity[0] += crandom() * 500 - (self->mass * 0.5);
			ent->avelocity[1] += crandom() * 500 - (self->mass * 0.5);
			ent->avelocity[2] += crandom() * 500 - (self->mass * 0.5);
		}
		if (strcmp(ent->classname, "gibx") != 0)
		{
			VectorAdd(self->velocity, dir, self->velocity);
			if (!self->groundentity)
				self->velocity[2] += 50;
		}
	
		//if(ent->groundentity)
		//	ent->s.origin[2]++;
		
		if(ent->groundentity)
			ent->velocity[2] += 200;

		
		if (ent->spawnflags & 2)
			mod = MOD_HELD_GRAVITY_GRENADE;
		else if (ent->spawnflags & 1)
			mod = MOD_GRAVITY_GRENADE;
		//gi.bprintf(PRINT_HIGH, "normal = %f %f %f", dir_copy[0], dir_copy[1], dir_copy[2]);
		
		VectorCopy(dir_copy, dir);
		VectorNormalize(dir);
		
		T_Damage(ent, self, self->owner, dir_copy, ent->s.origin, dir, (int)((VectorLength(dir_copy) / 1000) * self->dmg), 1, DAMAGE_NO_KNOCKBACK, mod);

		
	}

	if (self->delay < level.time)
	{
		
		self->dmg = 25;
		self->think = Grenade_Explode;
	}




}

void Grenade_Gravity_start(edict_t *self)
{
	M_avoid_danger(self);
	if (self->delay < level.time)
	{
		self->think = Grenade_Gravity;
		self->delay = level.time + 6;
	}
	self->nextthink = level.time + FRAMETIME;

}

void fire_grenade2(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held, int gravity)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;



	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);

	if (self->client)
		check_dodge(self, start, dir, speed, DODGE_PRIORITY_HIGH);

	grenade = G_Spawn();
	VectorCopy(start, grenade->s.origin);
	VectorScale(aimdir, speed, grenade->velocity);
	VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	if (gi.pointcontents(start) & MASK_WATER)
		VectorScale(grenade->velocity, 0.33, grenade->velocity);
	VectorSet(grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear(grenade->mins);
	VectorClear(grenade->maxs);
	VectorSet(grenade->mins, -2, -2, -2);
	VectorSet(grenade->maxs, 2, 2, 2);
	grenade->s.modelindex = gi.modelindex("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->delay = level.time + timer;
	grenade->nextthink = level.time + FRAMETIME;
	if (gravity)
	{
		grenade->think = Grenade_Gravity_start;
		grenade->touch = GrenadeGravity_Touch;
		grenade->delay = grenade->nextthink + 6;
		grenade->nextthink += 0.1;
		grenade->bounce_amount = BOUNCE_LOWER;
		//gi.bprintf(PRINT_HIGH, "gravity grenade = %i", gravity);
		if (gravity == 3)
		{
			
			grenade->dmg = damage * 5;
			grenade->dmg = damage;
			grenade->dmg_radius = damage_radius / 1.5;
		}
		else
		{
			grenade->dmg_radius = damage_radius * 2;
			grenade->dmg = damage * 0.05;
		}
		if (held)
		VectorClear(grenade->velocity);
			
	}
	else
	{
		grenade->touch = Grenade_Touch;
		grenade->think = grenade_think;
		grenade->dmg = damage;
		grenade->dmg_radius = damage_radius;
		
	}

	grenade->movetype = MOVETYPE_BOUNCE;

	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0 && !gravity)
		Grenade_Explode(grenade);
	else
	{
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity(grenade);
	}

}


/*
=================
fire_rocket
=================
*/
qboolean check_point(vec3_t origin, vec3_t dest)
{
	if (gi.pointcontents(dest));
	{
		trace_t tr;
		tr = gi.trace(origin, NULL, NULL, dest, NULL, MASK_SOLID);
		if (tr.fraction < 1)
		{
			VectorCopy(tr.endpos, origin);
			while (1)
			{
				tr = gi.trace(origin, NULL, NULL, dest, NULL, MASK_SOLID);

			}
		}
	}
	return true;
}
void smodel_think(edict_t *self)
{
	self->nextthink = level.time + 0.1;

	M_avoid_danger(self);

	if (self->s.skinnum < 7)
		self->s.skinnum++;
	//if (self->s.frame == 4)
	//	self->s.renderfx |= RF_TRANSLUCENT;

	//T_RadiusDamage(self, self->owner, 50, self, (1 + (self->s.frame * 8)) + 512, MOD_R_SPLASH);
	//gi.bprintf(PRINT_HIGH, "owner = %s\n", self->owner->classname);

	if (self->s.frame == self->noise_index2)
		G_FreeEdict(self);

	self->s.frame++;

	vec3_t dist;
	edict_t *ent = NULL;
	self->velocity[0] *= 1 + (crandom() / 4);
	self->velocity[1] *= 1 + (crandom() / 4);
	self->velocity[2] *= 1 + (crandom() / 4);
	while ((ent = findradius(ent, self->s.origin, (1 + (self->s.frame * 4)) + 256)) != NULL)
	{



		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0) && (strcmp(ent->classname, "subexpl") != 0) && (strcmp(ent->classname, "smodel") != 0))
			continue;
		if (!self->s.skinnum)
			continue;
		VectorSubtract(self->s.origin, ent->s.origin, dist);
		if ((ent->takedamage))
			T_Damage(ent, self, self->owner, self->velocity, self->s.origin, self->s.origin, (int)(100 / (VectorLength(dist) / 4)), 1, DAMAGE_NO_KNOCKBACK, MOD_R_SPLASH);





	}

}
void subexpl_think(edict_t *ent)
{

	M_avoid_danger(ent);

	edict_t *smodel;
	smodel = G_Spawn();
	VectorCopy(ent->s.origin, smodel->s.origin);
	smodel->solid = SOLID_NOT;
	smodel->movetype = MOVETYPE_BOUNCE;
	smodel->clipmask = (MASK_SHOT | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER);
	VectorCopy(ent->velocity, smodel->velocity);
	VectorScale(smodel->velocity, 0.5, smodel->velocity);
	smodel->nextthink = level.time + 0.1;
	smodel->think = smodel_think;
	smodel->classname = "smodel";
	smodel->s.effects |= EF_ROCKET;

	if (rand() % 100 < 50)
	{
		smodel->s.frame = 15;
		smodel->noise_index2 = 29;
	}
	else if (rand() % 100 > -1)
	{
		smodel->s.frame = 30; 
		smodel->noise_index2 = 47;
	}
	else
		smodel->noise_index2 = 15;

	smodel->s.modelindex = gi.modelindex("models/objects/r_explode/tris_hb.md2");
	smodel->gravity = -0.1;
	VectorClear(smodel->mins);
	VectorClear(smodel->maxs);
	vectoangles(smodel->velocity, smodel->s.angles);
	smodel->velocity[0] *= 1 + (crandom() / 3);
	smodel->velocity[1] *= 1 + (crandom() / 3);
	smodel->velocity[2] *= 1 + (crandom() / 3);
	smodel->s.renderfx |= (RF_NOSHADOW | RF_MINLIGHT);
	smodel->s.renderfx |= RF_TRANSLUCENT;
	smodel->owner = ent->owner;
	ent->nextthink = level.time + ((rand() % 5 + 1) * 0.1);

	ent->count++;

	/*gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION1);
	gi.WritePosition(ent->s.origin);
	gi.multicast(ent->s.origin, MULTICAST_PVS);*/
	int random = rand() % 5;
	if (random == 1)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/GRENLX1A.WAV"), 1, ATTN_NORM, 0);
	if (random == 2)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/ROCKLX1A.WAV"), 1, ATTN_NORM, 0);
	if (random == 3)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("world/world/explod2.wav"), 1, ATTN_NORM, 0);

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, ent, ent->dmg_radius, MOD_R_SPLASH);

	if (ent->count > 3) //3
		G_FreeEdict(ent);

}
void nuke_endgame(edict_t *self)
{
	gi.AddCommandString("gamemap vnuke.pcx");
}
void rocket_heatbomb(edict_t *ent)
{
	edict_t *subexpl;
	vec3_t dir;
	int index = -1;
	float speed;
	int radius_damage = 100;
	int damage_radius = 100;
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("world/explod1.wav"), 1, ATTN_NORM, 0);


	M_avoid_danger(ent);


repeat:
	index++;

	if (rand() % 2 == 1)
		goto repeat;
	speed = 256 + (crandom() * 128);
	subexpl = G_Spawn();
	VectorCopy(ent->s.origin, subexpl->s.origin);
	vectoangles(hb_dir[index], subexpl->s.angles);
	VectorScale(hb_dir[index], speed, subexpl->velocity);
	subexpl->gravity = 0.1 + (random() / 3);
	subexpl->movetype = MOVETYPE_BOUNCE;
	subexpl->clipmask = (MASK_SHOT | CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER);
	subexpl->solid = SOLID_BBOX;
	subexpl->s.effects |= EF_BLASTER;
	subexpl->s.effects |= EF_GRENADE;
	subexpl->s.effects |= EF_GREENGIB;
	subexpl->s.effects |= EF_ROCKET;
	if (rand() % 3 == 1|| 1==1)
	{
		subexpl->takedamage = DAMAGE_YES;
		subexpl->health = 99999;
		subexpl->flags |= FL_GODMODE;
	}
	VectorClear(subexpl->mins);
	VectorClear(subexpl->maxs);
	subexpl->s.modelindex = gi.modelindex("models/objects/debris2_sm/tris.md2");
	//subexpl->s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
	subexpl->owner = ent->owner;
	subexpl->think = subexpl_think;
	//subexpl->touch = rocket_touch;
	subexpl->nextthink = level.time + ((rand() % 5 + 1) * 0.1);
	subexpl->radius_dmg = radius_damage;
	subexpl->dmg_radius = damage_radius;
	//subexpl->s.sound = gi.soundindex("weapons/rockfly.wav");
	subexpl->classname = "subexpl";
	if (ent->flags & FL_NUKE && !index)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("world/battle2.wav"), 1, ATTN_NORM, 0);

	}
	if (index < 25)
		goto repeat;
	else if (ent->flags & FL_NUKE)
	{
		ent->think = nuke_endgame;
		ent->nextthink = level.time + 2;
		
	}
	else
		G_FreeEdict(ent);

}
void rocket_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA(ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage(other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);

	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP | SURF_TRANS33 | SURF_TRANS66 | SURF_FLOWING)))
			{
				n = rand() % 3;
				while (n--)
					ThrowDebris(ent, "models/objects/debris2/tris.md2", 50, ent->s.origin, NULL);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte(svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte(TE_ROCKET_EXPLOSION);
	gi.WritePosition(origin);
	gi.multicast(ent->s.origin, MULTICAST_PHS);
	if (ent->spawnflags == 1)
		rocket_heatbomb(ent);
	G_FreeEdict(ent);
}

void throw_nuke(edict_t *self)
{
	edict_t *nuke;
	vec3_t forward;
	nuke = G_Spawn();
	VectorCopy(self->s.origin, nuke->s.origin);
	nuke->owner = self;
	VectorSet(self->mins, -4, -4, -4);
	VectorSet(self->maxs, 4, 4, -5);
	nuke->clipmask = MASK_SHOT;
	nuke->solid = SOLID_BBOX;
	nuke->mass = 100;
	nuke->think = rocket_heatbomb;
	nuke->nextthink = level.time + 9;
	nuke->s.sound = gi.soundindex("world/klaxon1.wav");
	nuke->s.modelindex = gi.modelindex("models/items/ammo/nuke/tris.md2");
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorScale(forward, 300, nuke->velocity);
	nuke->velocity[2] += 200;
	nuke->flags |= FL_NUKE;

	gi.linkentity(nuke);
}

void rocket_think(edict_t *self)
{
	if (self->delay < level.time)
		G_FreeEdict(self);
	self->nextthink = level.time + FRAMETIME;
	VectorScale(self->velocity, 1.1, self->velocity);

	M_avoid_danger(self);

}

void fire_rocket(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	silencer_degrade(self, damage);

	rocket = G_Spawn();
	VectorCopy(start, rocket->s.origin);
	VectorCopy(dir, rocket->movedir);
	vectoangles(dir, rocket->s.angles);
	VectorScale(dir, speed * 0.66, rocket->velocity);
	if (gi.pointcontents(start) & (CONTENTS_LAVA | CONTENTS_WATER | CONTENTS_SLIME))
		VectorScale(rocket->velocity, 0.33, rocket->velocity);
	if (!strcmp(self->classname, "monster_infantry"))
		VectorMA(rocket->s.origin, 10, dir, rocket->s.origin);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear(rocket->mins);
	VectorClear(rocket->maxs);
	VectorSet(rocket->mins, -3, -1, -1);
	VectorSet(rocket->maxs, 3, 1, 1);
	rocket->s.modelindex = gi.modelindex("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + FRAMETIME;
	rocket->delay = level.time + 8000 / speed;

	rocket->think = rocket_think;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex("weapons/rockfly.wav");
	rocket->classname = "rocket";
	if (self->client && self->client->pers.weapon_ext.scounter)
	{
		rocket->spawnflags = 1;
		self->client->pers.weapon_ext.scounter = 0;
	}
	else if (self->monsterinfo.aiflags & AI_SHOOTSROCKET)
	{
		rocket->spawnflags = 1;
	}
	if (self->client)
		check_dodge(self, rocket->s.origin, dir, speed, DODGE_PRIORITY_HIGH);

	gi.linkentity(rocket);
}

/*
=================
fire_rail
=================
*/
void fire_rail(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask, i = 0;
	qboolean	water;

	silencer_degrade(self, damage);

	if (self->client)
		check_dodge(self, start, aimdir, 9999, DODGE_PRIORITY_HIGH);
	VectorMA(start, 8192, aimdir, end);
	VectorCopy(start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA;
	if (self->client && self->client->buttonsx & BUTTON_ATTACK2 || (self->svflags & SVF_MONSTER) && (self->monsterinfo.aiflags & AI_SHOOTRAILGUNFLAK))
	{
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		ThrowDebris(self, "models/objects/debris2_sm/tris.md2", WEAPON_RAILGUN_FLAKCANNON_SPEED, start, aimdir);
		if (self->svflags & SVF_MONSTER)
			self->monsterinfo.aiflags &= ~AI_SHOOTRAILGUNFLAK;
	}
	else
	{
		while (ignore && i < 256)	// Knightmare- fix infinite loop bug
		{
			tr = gi.trace(from, NULL, NULL, end, ignore, mask);

			if (tr.contents & (CONTENTS_SLIME | CONTENTS_LAVA))
			{
				mask &= ~(CONTENTS_SLIME | CONTENTS_LAVA);
				water = true;
			}
			else
			{
				//ZOID--added so rail goes through SOLID_BBOX entities (gibs, etc)
				if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client) ||
					(tr.ent->solid == SOLID_BBOX))
					ignore = tr.ent;
				else
					ignore = NULL;

				if ((tr.ent != self) && (tr.ent->takedamage))
					T_Damage(tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
			}

			VectorCopy(tr.endpos, from);
			i++;	// Knightmare added
		}

		// send gun puff / flash
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_RAILTRAIL);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
		gi.multicast(self->s.origin, MULTICAST_PHS);
		//	gi.multicast (start, MULTICAST_PHS);
		if (water)
		{
			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_RAILTRAIL);
			gi.WritePosition(start);
			gi.WritePosition(tr.endpos);
			gi.multicast(tr.endpos, MULTICAST_PHS);
		}
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_bfg
=================
*/
void bfg_explode(edict_t *self)
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
			if (!CanDamage(ent, self))
				continue;
			if (!CanDamage(ent, self->owner))
				continue;

			VectorAdd(ent->mins, ent->maxs, v);
			VectorMA(ent->s.origin, 0.5, v, v);
			VectorSubtract(self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist / self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_EXPLOSION);
			gi.WritePosition(ent->s.origin);
			gi.multicast(ent->s.origin, MULTICAST_PHS);
			T_Damage(ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	// core explosion - prevents firing it into the wall/floor
	if (self->spawnflags == 1)
	{
		if (other->takedamage)
			T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200 / 4, 0, 0, MOD_BFG_BLAST);
		T_RadiusDamage(self, self->owner, 200 / 4, other, 100, MOD_BFG_BLAST);

	}
	else
	{
		if (other->takedamage)
			T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
		T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);
	}
	gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA(self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear(self->velocity);
	self->s.modelindex = gi.modelindex("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte(svc_temp_entity);
	if (!self->spawnflags)
		gi.WriteByte(TE_BFG_BIGEXPLOSION);
	else
		gi.WriteByte(TE_BFG_EXPLOSION);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

}


void bfg_think(edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	M_avoid_danger(self);

	if (self->spawnflags != 1)
	{
		ent = NULL;

		while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
		{
			if (ent == self)
				continue;

			if (ent == self->owner)
				continue;

			if (!ent->takedamage)
				continue;

			if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
				continue;

			VectorMA(ent->absmin, 0.5, ent->size, point);

			VectorSubtract(point, self->s.origin, dir);
			VectorNormalize(dir);

			ignore = self;
			VectorCopy(self->s.origin, start);
			VectorMA(start, 2048, dir, end);
			while (1)
			{
				tr = gi.trace(start, NULL, NULL, end, ignore, CONTENTS_SOLID | CONTENTS_MONSTER | CONTENTS_DEADMONSTER);

				if (!tr.ent)
					break;

				// hurt it if we can
				if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
					T_Damage(tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

				// if we hit something that's not a monster or player we're done
				if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
				{
					gi.WriteByte(svc_temp_entity);
					gi.WriteByte(TE_LASER_SPARKS);
					gi.WriteByte(4);
					gi.WritePosition(tr.endpos);
					gi.WriteDir(tr.plane.normal);
					gi.WriteByte(self->s.skinnum);
					gi.multicast(tr.endpos, MULTICAST_PVS);
					break;
				}

				ignore = tr.ent;
				VectorCopy(tr.endpos, start);
			}

			gi.WriteByte(svc_temp_entity);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(self->s.origin);
			gi.WritePosition(tr.endpos);
			gi.multicast(self->s.origin, MULTICAST_PHS);
		}
	}
	self->nextthink = level.time + FRAMETIME;
}


void fire_bfg(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	silencer_degrade(self, damage);

	bfg = G_Spawn();
	VectorCopy(start, bfg->s.origin);
	VectorCopy(dir, bfg->movedir);
	vectoangles(dir, bfg->s.angles);
	VectorScale(dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear(bfg->mins);
	VectorClear(bfg->maxs);
	//VectorSet(bfg->mins, -1, -1, -1);
	//VectorSet(bfg->maxs, 1, 1, 1);

	bfg->s.modelindex = gi.modelindex("sprites/s_bfg1.sp2");
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000 / speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex("weapons/bfg__l1a.wav");
	if (speed == 1500)
		bfg->spawnflags = 1;
	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	//bfg->teammaster = bfg;
	bfg->teamchain = NULL;

	if (self->client)
		check_dodge(self, bfg->s.origin, dir, speed, DODGE_PRIORITY_HIGH);

	gi.linkentity(bfg);
}

void emp_effect(edict_t *self)
{
	trace_t tr;
	vec3_t dest;
	edict_t *ignore;
	ignore = self;
	int loop_counter = 0;
	int hit_something = 0;


fire:
	VectorCopy(self->s.origin, dest);
	dest[0] += crandom() * 32;
	dest[1] += crandom() * 32;
	dest[2] += crandom() * 32;

	
	tr = gi.trace(self->s.origin, NULL, NULL, dest, ignore, MASK_SHOT);
	

		if ((tr.ent->client || tr.ent->monsterinfo.scale) && tr.ent->takedamage && strcmp(tr.ent->classname, "worldspawn") != 0)
		{
			vec3_t dir;
			VectorSubtract(self->s.origin, tr.ent->s.origin, dir);
			//self->owner == self->owner_solid;
			T_Damage(tr.ent, self, self->owner_solid, dir, tr.endpos, vec3_origin, 1, 1, DAMAGE_ENERGY, MOD_BFG_LASER);
			if (tr.ent->client)
			{
				int ix = ITEM_INDEX(FindItem("Cells"));
				if (tr.ent->client->pers.inventory[ix])
					tr.ent->client->pers.inventory[ix] = clamp(tr.ent->client->pers.inventory[ix] - 15, tr.ent->client->pers.inventory[ix], 0);

			}
			//self->owner = 0;
			hit_something = 1;
			if (tr.ent->monsterinfo.scale && strcmp(tr.ent->classname, "monster_mutant") != 0 && strcmp(tr.ent->classname, "monster_flipper") != 0)
				tr.ent->monsterinfo.emp_effect_left += EMP_STUN_ADD;
			//gi.bprintf(PRINT_HIGH, "emp hit %s", tr.ent->classname);
		}

		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_WELDING_SPARKS);
		gi.WriteByte(25);
		gi.WritePosition(tr.endpos);
		gi.WriteDir(tr.plane.normal);
		gi.WriteByte(0xe0); 
		gi.multicast(tr.endpos, MULTICAST_PVS);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_WELDING_SPARKS);
		gi.WriteByte(5);
		gi.WritePosition(tr.endpos);
		gi.WriteDir(tr.plane.normal);
		gi.WriteByte((int)"06f0");
		gi.multicast(tr.endpos, MULTICAST_PVS);

		VectorCopy(tr.endpos, self->s.origin);
		loop_counter++;
		if (hit_something && loop_counter < 100)
		{
			//gi.bprintf(PRINT_HIGH, "!!!!!!!!!!!!!!!!!!!!!!EMP HIT = %s, loop_counter = %i!!!!!!!!!!!!!!!!!!!!!!\n", tr.ent->classname, loop_counter);
			ignore = tr.ent;
			hit_something = 0;
			goto fire;
		}
		if (!self->s.modelindex)
		{
			switch (rand() % 8)
			{
			case 0:
			case 1:
				self->s.modelindex = gi.modelindex("sprites/emp_sm1.sp2");
				break;
			case 2:
			case 3:
				self->s.modelindex = gi.modelindex("sprites/emp_sm2.sp2");
				break;
			case 4:
			case 5:
				self->s.modelindex = gi.modelindex("sprites/emp_sm3.sp2");
				break;
			case 6:
			case 7:
				self->s.modelindex = gi.modelindex("sprites/emp_sm4.sp2");
				break;
			} 
		}
		else
		{
			self->s.modelindex = 0;
		}
	self->count++;
	if (self->count > 5 + rand() % 5)
		G_FreeEdict(self);
	self->nextthink = level.time + 0.1;

}
void emp_effect_spawn(edict_t *self)
{
	edict_t	*emp_effect_ent;
	
	emp_effect_ent = G_Spawn();
	emp_effect_ent->classname = "EMP_effect";

		emp_effect_ent->owner_solid = self->owner_solid;
		VectorCopy(self->s.origin, emp_effect_ent->s.origin);
		emp_effect_ent->think = emp_effect;
		emp_effect_ent->nextthink = level.time + 0.1;
		emp_effect_ent->movetype = MOVETYPE_FLYMISSILE;
		emp_effect_ent->solid = SOLID_BBOX;
		emp_effect_ent->s.renderfx |= RF_TRANSLUCENT;
		//emp->s.effects |=  EF_ANIM_ALLFAST;
		VectorClear(emp_effect_ent->mins);
		VectorClear(emp_effect_ent->maxs);
		emp_effect(emp_effect_ent);
}
void emp_think(edict_t *self)
{
	self->s.angles[0] = crandom() * 180;
	self->s.angles[1] = crandom() * 180;
	self->s.angles[2] = crandom() * 180;
	self->nextthink = level.time + 0.1;
	if(self->count == 1)
		gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/emp_x.wav"), 1, ATTN_IDLE, 0);


	
		emp_effect_spawn(self);
		emp_effect_spawn(self);
		emp_effect_spawn(self);
		emp_effect_spawn(self);
		self->count++;
		self->s.frame++;

	
		if (self->count == 4)
		{
			self->s.modelindex = 0; 
			G_FreeEdict(self);
		}

		M_avoid_danger(self);


}
void fire_emp(edict_t *self, vec3_t start, vec3_t aimdir, int damage)
{
	edict_t	*emp;
	vec3_t end;
	trace_t tr;
	VectorMA(start, 512, aimdir, end);

	silencer_degrade(self, damage);

	tr = gi.trace(start, NULL, NULL, end, self, (MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA));

		emp = G_Spawn();
		emp->classname = "EMP_main";
		VectorCopy(tr.endpos, emp->s.origin);
		emp->owner_solid = self;
		emp->think = emp_think;
		emp->nextthink = level.time + 0.1;
		emp->count = 1;
		//emp->s.modelindex = gi.modelindex("sprites/s_bfg1.sp2");
		if(rand() % 2 == 1)
		emp->s.modelindex = gi.modelindex("sprites/emp1x.sp2");
		else
		emp->s.modelindex = gi.modelindex("sprites/emp2x.sp2");
		emp->movetype = MOVETYPE_FLYMISSILE;
		//emp->clipmask = MASK_SHOT;
		emp->solid = SOLID_BBOX;
		emp->s.renderfx |= RF_TRANSLUCENT;
		emp->s.effects |=  EF_ANIM_ALLFAST;
		VectorClear(emp->mins);
		VectorClear(emp->maxs);

		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(emp - g_edicts);
		gi.WriteByte(MZ_NUKE2);
		gi.multicast(emp->s.origin, MULTICAST_PVS);

		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(emp - g_edicts);
		gi.WriteByte(MZ_NUKE4);
		gi.multicast(emp->s.origin, MULTICAST_PVS);
	//	if (tr.ent)
		//	gi.bprintf(PRINT_HIGH, "EMP: HIT %s\n", tr.ent->classname);
		//back off a bit if we didn't hit anyone
		if (!tr.ent->client && !tr.ent->monsterinfo.scale)
		{
			VectorMA(emp->s.origin, -EMP_BACK_OFF, aimdir, emp->s.origin);
			//and raise up a bit if neccessary
			VectorCopy(emp->s.origin, aimdir);
			//gi.bprintf(PRINT_HIGH, "EMP:BACKING OFF\n");

			tr = gi.trace(emp->s.origin, NULL, NULL, aimdir, self, (MASK_SHOT | CONTENTS_SLIME | CONTENTS_LAVA));

			emp->s.origin[2] = (EMP_BACK_OFF - fabs(tr.endpos[2] - emp->s.origin[2])) + emp->s.origin[2];
			//gi.bprintf(PRINT_HIGH, "EMP: MOVING UP\n");

		}


		gi.linkentity(self);
	//gi.bprintf(PRINT_HIGH, "FIRE EMP\n");
}




void fire_emp_b(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles(aimdir, dir);
	AngleVectors(dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy(start, grenade->s.origin);
	VectorScale(aimdir, speed, grenade->velocity);
	VectorMA(grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA(grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet(grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear(grenade->mins);
	VectorClear(grenade->maxs);
	grenade->s.modelindex = gi.modelindex("models/objects/grenade_h/tris.md2");
	grenade->owner = self;
	//grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + 0.1;
	grenade->think = emp_think;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "emp_blast";
	grenade->delay = 8 + level.time;
	grenade->gravity = 0.25;
	//grenade->s.modelindex = gi.modelindex("sprites/emp1x.sp2");



	gi.linkentity(grenade);
}