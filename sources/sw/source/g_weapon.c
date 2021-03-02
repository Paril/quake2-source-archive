#include "g_local.h"
#include "m_player.h"

void MMMDonut (edict_t *ent);

void SWTC_MuzzleFlash (edict_t *ent, byte effect)
{
	if(ent->client->chasetoggle == 1)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (effect);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (effect);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
}

/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/

void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
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
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;
	return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
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
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/

#define EF_SABER	4

void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	int	fire_frames1[] = {21, 22, 23, 0};
	int		mod, damage, invisi;
	float r, u, n;
	vec3_t	forward, right, up, start, offset;
	char *weap;
	qboolean block;

	if (other == self->owner)
		return;

	if(other->s.renderfx & RF_BEAM)
		return;

	if(other->client)
	{
		if(Force_constant_active(other, LFORCE_INVISIBILITY) != 255 && other->client->invisi_time < level.time)
		{
			invisi = 1;
		}
		else
		{
			invisi = 0;
		}

		if((other->client) && (Force_constant_active(other, LFORCE_SHIELD) != 255))
		{
			damage = ceil((self->dmg * 0.75)/10);
			other->client->pers.force.pool -= damage;
			gi.dprintf("draining %in", damage);
			damage = self->dmg * 0.25;
		}
		else
		{
			damage = self->dmg;
		}
	}
	else
	{
		invisi = 0;
		damage = self->dmg;
	}

	if(self->spawnflags & 1)
		mod = MOD_RIFLE;
	else
		mod = MOD_BLASTER;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage && other->client)
	{
		if(other->client->pers.weapon)
			weap = other->client->pers.weapon->classname;
		else
			weap = "";
		if((Force_constant_active(other, LFORCE_REFLECT) != 255) && (!invisi))
		{
			if(infront (other, self->owner))
			{
				self->owner = other;
				AngleVectors (other->s.angles, forward, right, up);

				VectorSet(offset, 24, 8, other->viewheight-8);
				G_ProjectSource (other->s.origin, offset, forward, right, start);

				r = crandom()*0.25;
				u = crandom()*0.2;

				VectorMA (forward, r, right, forward);
				VectorMA (forward, u, up, forward);

				G_FreeEdict (self);

				if(mod == MOD_RIFLE)
					fire_blaster(other, start, forward, damage, 2048, true);
				else
					fire_blaster(other, start, forward, damage, 2048, false);

				return;
			}
		}
		else if(advanced->value)
		{
			if ((strcmp(weap, "weapon_saber") == 0) && (!invisi) && (other->client->duel))
			{
				if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
				{
					block = 1;
				}

				if (other->client->weaponstate == WEAPON_READY || block)
				{
					if(infront (other, self->owner))
					{
						self->owner = other;
						AngleVectors (other->s.angles, forward, right, up);

						VectorSet(offset, 24, 8, other->viewheight-8);
						G_ProjectSource (other->s.origin, offset, forward, right, start);

						r = crandom()*0.25;
						u = crandom()*0.2;

						VectorMA (forward, r, right, forward);
						VectorMA (forward, u, up, forward);

						G_FreeEdict (self);

						if(mod == MOD_RIFLE)
							fire_blaster(other, start, forward, damage, 2048, true);
						else
							fire_blaster(other, start, forward, damage, 2048, false);

						other->client->buttons |= BUTTON_ATTACK;

						n = random();

						if(random() > 0.8)
						{
							other->client->swing_num = BLOCK_LEFT;
						}
						else if(random() > 0.6)
						{
							other->client->swing_num = BLOCK_RIGHT;
						}
						else if(random() > 0.45)
						{
							other->client->swing_num = BLOCK_UPRIGHT;
						}
						else if (random() > 0.3)
						{
							other->client->swing_num = BLOCK_UPLEFT;
						}
						else if(random() > 0.15)
						{
							other->client->swing_num = BLOCK_LOWRIGHT;
						}
						else
						{
							other->client->swing_num = BLOCK_LOWLEFT;
						}

						other->client->swing_frame = 1;
						Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

						return;
					}
				}
			}
		}
		else if ((strcmp(weap, "weapon_saber") == 0) && (!invisi))
		{
			if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
			{
				block = 1;
			}
			if (other->client->weaponstate == WEAPON_READY || block)
			{
				if(infront (other, self->owner))
				{
					self->owner = other;
					AngleVectors (other->s.angles, forward, right, up);

					VectorSet(offset, 24, 8, other->viewheight-8);
					G_ProjectSource (other->s.origin, offset, forward, right, start);

					r = crandom()*0.25;
					u = crandom()*0.2;

					VectorMA (forward, r, right, forward);
					VectorMA (forward, u, up, forward);

					G_FreeEdict (self);

					if(mod == MOD_RIFLE)
						fire_blaster(other, start, forward, damage, 2048, true);
					else
						fire_blaster(other, start, forward, damage, 2048, false);

					other->client->buttons |= BUTTON_ATTACK;

					n = random();

					if(random() > 0.8)
					{
						other->client->swing_num = BLOCK_LEFT;
					}
					else if(random() > 0.6)
					{
						other->client->swing_num = BLOCK_RIGHT;
					}
					else if(random() > 0.45)
					{
						other->client->swing_num = BLOCK_UPRIGHT;
					}
					else if (random() > 0.3)
					{
						other->client->swing_num = BLOCK_UPLEFT;
					}
					else if(random() > 0.15)
					{
						other->client->swing_num = BLOCK_LOWRIGHT;
					}
					else
					{
						other->client->swing_num = BLOCK_LOWLEFT;
					}

					other->client->swing_frame = 1;
					Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

					return;
				}
			}
		}
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}
	else if(other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
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

void blaster_think (edict_t *ent)
{
	vec3_t start, end;
	trace_t tr;

	VectorCopy(ent->s.origin, start);

	VectorMA(start, ent->speed, ent->movedir, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL);

	if(tr.fraction == 1)
	{
		VectorCopy(ent->s.origin, ent->s.old_origin);
		VectorCopy(end, ent->s.origin);
		ent->nextthink = level.time + 0.01;
		gi.linkentity(ent);
	}
	else
	{
		VectorCopy(tr.endpos, ent->s.origin);
		ent->touch (ent, tr.ent, &tr.plane, tr.surface);
	}
}

//BLUE		f0 - f2
//GREEN	f3 - f5
//RED		f6 - f8
//YELLOW	0xdcdddedf;
//ORANGE	0xe0e1e2e3;
//PINK		0xffffffff;

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, qboolean trooper)
{
	edict_t *bolt;
	trace_t tr;
	vec3_t	spot, endspot;

//	if(trooper)
//		SWTC_MuzzleFlash (self, MZ_RIFLE);
//	else
//		SWTC_MuzzleFlash (self, MZ_PISTOL);

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	VectorAdd (start, bolt->velocity, bolt->s.old_origin);
	bolt->clipmask = MASK_SHOT;

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_BBOX;
	bolt->s.renderfx |= RF_BEAM;
	bolt->s.modelindex = 1;
	bolt->owner = self;

	if(trooper)
	{
		bolt->classname = "E11 laser";
		bolt->spawnflags |= 1;
	}
	else
	{
		bolt->classname = "DL44 Laser";
	}

	bolt->s.frame = 3;

	if(trooper)
		bolt->s.skinnum = 0xf8f7f7f6;
	else
		bolt->s.skinnum = 0xf2f1f1f0; //blue/green

	VectorSet (bolt->mins, -3, -3, -3);
	VectorSet (bolt->maxs, 3, 3, 3);

	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 20;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->radius_dmg = damage;
	bolt->dmg_radius = 32;
	
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	VectorCopy(self->s.origin, spot);
	spot[2] += self->viewheight;

	VectorMA(spot, 64, dir, endspot);

	tr = gi.trace (spot, NULL, NULL, endspot, self, bolt->clipmask);
	
	if (tr.fraction != 1)
	{
//		gi.dprintf("fraction\n");
		VectorCopy(tr.endpos, bolt->s.origin);
		bolt->touch (bolt, tr.ent, &tr.plane, tr.surface);
	}
}

/*
=================
fire_disruptor

Fires disruptor bolt. Black death baby!.
=================
*/
void disruptor_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int	damage, invisi;

	float r, u, n;
	vec3_t	forward, right, up, start, offset;
	char *weap;
	qboolean block;

	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	int	fire_frames1[] = {21, 22, 23, 0};

	if (other == self->owner)
		return;

	if(other->s.renderfx & RF_BEAM)
		return;

	if(other->client)
	{
		if(Force_constant_active(other, LFORCE_INVISIBILITY) != 255 && other->client->invisi_time < level.time)
		{
			invisi = 1;
		}
		else
		{
			invisi = 0;
		}

		if((other->client) && (Force_constant_active(other, LFORCE_SHIELD) != 255))
		{
			damage = ceil((self->dmg * 0.75)/10);
			other->client->pers.force.pool -= damage;
			damage = self->dmg * 0.25;
		}
		else
		{
			damage = self->dmg;
		}
	}
	else
	{
		invisi = 0;
		damage = self->dmg;
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

//	Added Advanced duel mode parrying possibilities par public's request.
//	RipVTide

	if (other->takedamage && other->client)
	{
		if(other->client->pers.weapon)
			weap = other->client->pers.weapon->classname;
		else
			weap = "";
		if(advanced->value)
		{
			if ((strcmp(weap, "weapon_saber") == 0) && (!invisi) && (other->client->duel))
			{
				if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
				{
					block = 1;
				}

				if (other->client->weaponstate == WEAPON_READY || block)
				{
					if(infront (other, self->owner))
					{
						self->owner = other;
						AngleVectors (other->s.angles, forward, right, up);

						VectorSet(offset, 24, 8, other->viewheight-8);
						G_ProjectSource (other->s.origin, offset, forward, right, start);

						r = crandom()*0.25;
						u = crandom()*0.2;

						VectorMA (forward, r, right, forward);
						VectorMA (forward, u, up, forward);

						G_FreeEdict (self);

						fire_disruptor(other, start, forward, damage, 2048);

						other->client->buttons |= BUTTON_ATTACK;

						n = random();

						if(random() > 0.8)
						{
							other->client->swing_num = BLOCK_LEFT;
						}
						else if(random() > 0.6)
						{
							other->client->swing_num = BLOCK_RIGHT;
						}
						else if(random() > 0.45)
						{
							other->client->swing_num = BLOCK_UPRIGHT;
						}
						else if (random() > 0.3)
						{
							other->client->swing_num = BLOCK_UPLEFT;
						}
						else if(random() > 0.15)
						{
							other->client->swing_num = BLOCK_LOWRIGHT;
						}
						else
						{
							other->client->swing_num = BLOCK_LOWLEFT;
						}

						other->client->swing_frame = 1;
						Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

						return;
					}
				}
			}
		}
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_DISRUPTOR);
	}
	else if(other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_DISRUPTOR);
	}

	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_DISRUPTOR);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	G_FreeEdict (self);
}

void disruptor_think (edict_t *ent)
{
	vec3_t start, end;
	trace_t tr;

	VectorCopy(ent->s.origin, start);

	VectorMA(start, ent->speed, ent->movedir, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL);

	if(tr.fraction == 1)
	{
		VectorCopy(ent->s.origin, ent->s.old_origin);
		VectorCopy(end, ent->s.origin);
		ent->nextthink = level.time + 0.01;
		gi.linkentity(ent);
	}
	else
	{
		VectorCopy(tr.endpos, ent->s.origin);
		ent->touch (ent, tr.ent, &tr.plane, tr.surface);
	}
}

void fire_disruptor (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *bolt;
	trace_t tr;
	vec3_t	spot, end;

//	SWTC_MuzzleFlash (self, MZ_DISRUPTOR);

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->clipmask = MASK_SHOT;

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_TRACKER;
	bolt->s.renderfx |= RF_SHELL_HALF_DAM;
	bolt->s.modelindex = gi.modelindex("models/objects/disbolt/tris.md2");
	bolt->owner = self;

	bolt->s.frame = 0;

	// set the color
	bolt->s.skinnum = 0xffffffff; //pink

	VectorSet (bolt->mins, -3, -3, -3);
	VectorSet (bolt->maxs, 3, 3, 3);

	bolt->touch = disruptor_touch;
	bolt->nextthink = level.time + 20;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->radius_dmg = 100;
	bolt->dmg_radius = 64;
	
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	VectorCopy(self->s.origin, spot);
	spot[2] += self->viewheight;

	VectorMA(spot, 64, dir, end);

	tr = gi.trace (spot, NULL, NULL, end, self, bolt->clipmask);
	if (tr.fraction != 1)
	{
		VectorCopy(tr.endpos, bolt->s.origin);
		bolt->touch (bolt, tr.ent, &tr.plane, tr.surface);
	}

	gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/disrupt/fire.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(self, start, PNOISE_WEAPON);
}


/*
=================
fire_repeater
=================
*/
void repeater_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		damage, invisi;
	float r, u, n;
	vec3_t	forward, right, up, start, offset;
	char *weap;
	qboolean block;

	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	int	fire_frames1[] = {21, 22, 23, 0};

	if (other == self->owner)
		return;

	if(other->s.renderfx & RF_BEAM)
		return;

	if(other->client)
	{
		if(Force_constant_active(other, LFORCE_INVISIBILITY) != 255 && other->client->invisi_time < level.time)
		{
			invisi = 1;
		}
		else
		{
			invisi = 0;
		}

		if((other->client) && (Force_constant_active(other, LFORCE_SHIELD) != 255))
		{
			damage = ceil((self->dmg * 0.75)/10);
			other->client->pers.force.pool -= damage;
			damage = self->dmg * 0.25;
		}
		else
		{
			damage = self->dmg;
		}
	}
	else
	{
		invisi = 0;
		damage = self->dmg;
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage && other->client)
	{
		if(other->client->pers.weapon)
			weap = other->client->pers.weapon->classname;
		else
			weap = "";
		if((Force_constant_active(other, LFORCE_REFLECT) != 255) && (!invisi))
		{
			if(infront (other, self->owner))
			{
				self->owner = other;
				AngleVectors (other->s.angles, forward, right, up);

				VectorSet(offset, 24, 8, other->viewheight-8);
				G_ProjectSource (other->s.origin, offset, forward, right, start);

				r = crandom()*0.25;
				u = crandom()*0.2;

				VectorMA (forward, r, right, forward);
				VectorMA (forward, u, up, forward);

				G_FreeEdict (self);

				fire_repeater(other, start, forward, damage, 2048);

				return;
			}
		}
		else if(advanced->value)
		{
			if ((strcmp(weap, "weapon_saber") == 0) && (!invisi) && (other->client->duel))
			{
				if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
				{
					block = 1;
				}

				if (other->client->weaponstate == WEAPON_READY || block)
				{
					if(infront (other, self->owner))
					{
						self->owner = other;
						AngleVectors (other->s.angles, forward, right, up);

						VectorSet(offset, 24, 8, other->viewheight-8);
						G_ProjectSource (other->s.origin, offset, forward, right, start);

						r = crandom()*0.25;
						u = crandom()*0.2;

						VectorMA (forward, r, right, forward);
						VectorMA (forward, u, up, forward);

						G_FreeEdict (self);

						fire_repeater(other, start, forward, damage, 2048);

						other->client->buttons |= BUTTON_ATTACK;

						n = random();

						if(random() > 0.8)
						{
							other->client->swing_num = BLOCK_LEFT;
						}
						else if(random() > 0.6)
						{
							other->client->swing_num = BLOCK_RIGHT;
						}
						else if(random() > 0.45)
						{
							other->client->swing_num = BLOCK_UPRIGHT;
						}
						else if (random() > 0.3)
						{
							other->client->swing_num = BLOCK_UPLEFT;
						}
						else if(random() > 0.15)
						{
							other->client->swing_num = BLOCK_LOWRIGHT;
						}
						else
						{
							other->client->swing_num = BLOCK_LOWLEFT;
						}

						other->client->swing_frame = 1;
						Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

						return;
					}
				}
			}
		}
		else if ((strcmp(weap, "weapon_saber") == 0) && (!invisi))
		{
			if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
			{
				block = 1;
			}
			if (other->client->weaponstate == WEAPON_READY || block)
			{
				if(infront (other, self->owner))
				{
					self->owner = other;
					AngleVectors (other->s.angles, forward, right, up);

					VectorSet(offset, 24, 8, other->viewheight-8);
					G_ProjectSource (other->s.origin, offset, forward, right, start);

					r = crandom()*0.25;
					u = crandom()*0.2;

					VectorMA (forward, r, right, forward);
					VectorMA (forward, u, up, forward);

					G_FreeEdict (self);

					fire_repeater(other, start, forward, damage, 2048);

					other->client->buttons |= BUTTON_ATTACK;

					n = random();

					if(random() > 0.8)
					{
						other->client->swing_num = BLOCK_LEFT;
					}
					else if(random() > 0.6)
					{
						other->client->swing_num = BLOCK_RIGHT;
					}
					else if(random() > 0.45)
					{
						other->client->swing_num = BLOCK_UPRIGHT;
					}
					else if (random() > 0.3)
					{
						other->client->swing_num = BLOCK_UPLEFT;
					}
					else if(random() > 0.15)
					{
						other->client->swing_num = BLOCK_LOWRIGHT;
					}
					else
					{
						other->client->swing_num = BLOCK_LOWLEFT;
					}

					other->client->swing_frame = 1;
					Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

					return;
				}
			}
		}
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_REPEATER);
	}
	else if(other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_REPEATER);
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

//	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_REPEATER);

	G_FreeEdict (self);
}

void repeater_think (edict_t *ent)
{
	vec3_t start, end;
	trace_t tr;

	VectorCopy(ent->s.origin, start);

	VectorMA(start, ent->speed, ent->movedir, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL);

	if(tr.fraction == 1)
	{
		VectorCopy(ent->s.origin, ent->s.old_origin);
		VectorCopy(end, ent->s.origin);
		ent->nextthink = level.time + 0.01;
		gi.linkentity(ent);
	}
	else
	{
		VectorCopy(tr.endpos, ent->s.origin);
		ent->touch (ent, tr.ent, &tr.plane, tr.surface);
	}
}

void fire_repeater (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *bolt;
	trace_t tr;
	vec3_t	spot;

//	SWTC_MuzzleFlash (self, MZ_REPEATER);

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	VectorAdd (start, bolt->velocity, bolt->s.old_origin);
	bolt->clipmask = MASK_SHOT;

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_BBOX;
	bolt->s.renderfx |= RF_BEAM;
	bolt->s.modelindex = 1;
	bolt->owner = self;

	bolt->s.frame = 3;

	// set the color
	bolt->s.skinnum = 0xd0d1d2d3; //green

	VectorSet (bolt->mins, -3, -3, -3);
	VectorSet (bolt->maxs, 3, 3, 3);

	bolt->touch = repeater_touch;
	bolt->nextthink = level.time + 20;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->radius_dmg = 10;
	bolt->dmg_radius = 32;

	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	VectorCopy(self->s.origin, spot);
	spot[2] += self->viewheight;

	tr = gi.trace (spot, NULL, NULL, start, self, bolt->clipmask);
	if (tr.fraction != 1)
	{
		VectorCopy(tr.endpos, bolt->s.origin);
		bolt->touch (bolt, tr.ent, &tr.plane, tr.surface);
	}
}


/*
=================
fire_bowcaster
=================
*/
void bowcaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	int	fire_frames1[] = {21, 22, 23, 0};
	int		damage, invisi;
	float r, u, n;
	vec3_t	forward, right, up, start, offset;
	char *weap;
	qboolean block;

	if (other == self->owner)
		return;

	if(other->s.renderfx & RF_BEAM)
		return;

	if(other->client)
	{
		if(Force_constant_active(other, LFORCE_INVISIBILITY) != 255 && other->client->invisi_time < level.time)
		{
			invisi = 1;
		}
		else
		{
			invisi = 0;
		}

		if((other->client) && (Force_constant_active(other, LFORCE_SHIELD) != 255))
		{
			damage = ceil((self->dmg * 0.75)/10);
			other->client->pers.force.pool -= damage;
			damage = self->dmg * 0.25;
		}
		else
		{
			damage = self->dmg;
		}
	}
	else
	{
		invisi = 0;
		damage = self->dmg;
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage && other->client)
	{
		if(other->client->pers.weapon)
			weap = other->client->pers.weapon->classname;
		else
			weap = "";
		if((Force_constant_active(other, LFORCE_REFLECT) != 255) && (!invisi))
		{
			if(infront (other, self->owner))
			{
				self->owner = other;
				AngleVectors (other->s.angles, forward, right, up);

				VectorSet(offset, 24, 8, other->viewheight-8);
				G_ProjectSource (other->s.origin, offset, forward, right, start);

				r = crandom()*0.25;
				u = crandom()*0.2;

				VectorMA (forward, r, right, forward);
				VectorMA (forward, u, up, forward);

				G_FreeEdict (self);

				fire_bowcaster(other, start, forward, damage, 2048);

				return;
			}
		}
		else if(advanced->value)
		{
			if ((strcmp(weap, "weapon_saber") == 0) && (!invisi) && (other->client->duel))
			{
				if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
				{
					block = 1;
				}

				if (other->client->weaponstate == WEAPON_READY || block)
				{
					if(infront (other, self->owner))
					{
						self->owner = other;
						AngleVectors (other->s.angles, forward, right, up);

						VectorSet(offset, 24, 8, other->viewheight-8);
						G_ProjectSource (other->s.origin, offset, forward, right, start);

						r = crandom()*0.25;
						u = crandom()*0.2;

						VectorMA (forward, r, right, forward);
						VectorMA (forward, u, up, forward);

						G_FreeEdict (self);

						fire_bowcaster(other, start, forward, damage, 2048);

						other->client->buttons |= BUTTON_ATTACK;

						n = random();

						if(random() > 0.8)
						{
							other->client->swing_num = BLOCK_LEFT;
						}
						else if(random() > 0.6)
						{
							other->client->swing_num = BLOCK_RIGHT;
						}
						else if(random() > 0.45)
						{
							other->client->swing_num = BLOCK_UPRIGHT;
						}
						else if (random() > 0.3)
						{
							other->client->swing_num = BLOCK_UPLEFT;
						}
						else if(random() > 0.15)
						{
							other->client->swing_num = BLOCK_LOWRIGHT;
						}
						else
						{
							other->client->swing_num = BLOCK_LOWLEFT;
						}

						other->client->swing_frame = 1;
						Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

						return;
					}
				}
			}
		}
		else if ((strcmp(weap, "weapon_saber") == 0) && (!invisi))
		{
			if(other->s.frame >= FRAME_b_upleftA && random() >= 0.1)
			{
				block = 1;
			}
			if (other->client->weaponstate == WEAPON_READY || block)
			{
				if(infront (other, self->owner))
				{
					self->owner = other;
					AngleVectors (other->s.angles, forward, right, up);

					VectorSet(offset, 24, 8, other->viewheight-8);
					G_ProjectSource (other->s.origin, offset, forward, right, start);

					r = crandom()*0.25;
					u = crandom()*0.2;

					VectorMA (forward, r, right, forward);
					VectorMA (forward, u, up, forward);

					G_FreeEdict (self);

					fire_bowcaster(other, start, forward, damage, 2048);

					other->client->buttons |= BUTTON_ATTACK;

					n = random();

					if(random() > 0.8)
					{
						other->client->swing_num = BLOCK_LEFT;
					}
					else if(random() > 0.6)
					{
						other->client->swing_num = BLOCK_RIGHT;
					}
					else if(random() > 0.45)
					{
						other->client->swing_num = BLOCK_UPRIGHT;
					}
					else if (random() > 0.3)
					{
						other->client->swing_num = BLOCK_UPLEFT;
					}
					else if(random() > 0.15)
					{
						other->client->swing_num = BLOCK_LOWRIGHT;
					}
					else
					{
						other->client->swing_num = BLOCK_LOWLEFT;
					}

					other->client->swing_frame = 1;
					Weapon_Generic_Saber (other, 20, 28, 49, 60, pause_frames, fire_frames1, weapon_saber_fire);

					return;
				}
			}
		}
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_BOWCASTER);
	}
	else if(other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, MOD_BOWCASTER);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_HEATBEAM_STEAM);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

//	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_BOWCASTER);

	G_FreeEdict (self);
}

void bowcaster_think (edict_t *ent)
{
	vec3_t start, end;
	trace_t tr;

	VectorCopy(ent->s.origin, start);

	VectorMA(start, ent->speed, ent->movedir, end);

	tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL);

	if(tr.fraction == 1)
	{
		VectorCopy(ent->s.origin, ent->s.old_origin);
		VectorCopy(end, ent->s.origin);
		ent->nextthink = level.time + 0.01;
		gi.linkentity(ent);
	}
	else
	{
		VectorCopy(tr.endpos, ent->s.origin);
		ent->touch (ent, tr.ent, &tr.plane, tr.surface);
	}
}

void fire_bowcaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t *bolt;
	trace_t tr;
	vec3_t	spot;

//	SWTC_MuzzleFlash (self, MZ_BOWCASTER);

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	VectorAdd (start, bolt->velocity, bolt->s.old_origin);
	bolt->clipmask = MASK_SHOT;

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->solid = SOLID_BBOX;
	bolt->s.renderfx |= RF_BEAM;
	bolt->s.modelindex = 1;       
	bolt->owner = self;

	bolt->s.frame = 3;

	// set the color
	bolt->s.skinnum = 0xe0e1e2e3; //orange

	VectorSet (bolt->mins, -1, -1, -1);
	VectorSet (bolt->maxs, 1, 1, 1);

	bolt->touch = bowcaster_touch;
	bolt->nextthink = level.time + 20;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->radius_dmg = 10;
	bolt->dmg_radius = 32;
                
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	VectorCopy(self->s.origin, spot);
	spot[2] += self->viewheight;

	tr = gi.trace (spot, NULL, NULL, start, self, bolt->clipmask);
	if (tr.fraction != 1)
	{
		VectorCopy(tr.endpos, bolt->s.origin);
		bolt->touch (bolt, tr.ent, &tr.plane, tr.surface);
	}
}

/*
=================
fire_beamtube
=================
*/

void beamtube_hit (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf, vec3_t spot)
{
	vec3_t vel;

	VectorClear(vel);

	if (other == self)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		return;
	}

	if(other->s.renderfx & RF_BEAM)
		return;

	PlayerNoise(self, self->s.origin, PNOISE_IMPACT);

	if(other->takedamage)
	{
		T_Damage (other, self, self, vel, spot, plane->normal, 10, 1, DAMAGE_ENERGY, MOD_BEAMTUBE);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLECHETTE);
		gi.WritePosition (spot);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
}


void fire_beamtube (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	trace_t tr;
	vec3_t spot, forward;
	vec3_t offset, view;

//	SWTC_MuzzleFlash (self, MZ_BEAMTUBE);

	VectorCopy(self->s.origin, view);
	view[2] += self->viewheight;

	VectorSubtract(start, self->s.origin, offset);

	AngleVectors (self->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 4096, forward, spot);

	tr = gi.trace(view, NULL, NULL, start, self, MASK_SHOT);

	if(tr.fraction != 1)
	{
		beamtube_hit (self, tr.ent, &tr.plane, tr.surface, tr.endpos);
		return;
	}

	tr = gi.trace(start, NULL, NULL, spot, self, MASK_SHOT);

	VectorCopy(tr.endpos, spot);

//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_GRAPPLE_CABLE);
//	gi.WriteShort (self - g_edicts);
//	gi.WritePosition (start);
//	gi.WritePosition (spot);
//	gi.WritePosition (offset);
//	gi.multicast (start, MULTICAST_PVS);

	if(tr.fraction != 1)
	{
		beamtube_hit (self, tr.ent, &tr.plane, tr.surface, tr.endpos);
	}
}

/*
=================
fire_grenade
=================
*/
static void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_MISSILETUBE;
	else
		mod = MOD_THERMAL;

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

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

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_NUKEBLAST);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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

	ent->enemy = other;
	Grenade_Explode (ent);
}

void fire_thermal (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held, int secfunc)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

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
//	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/td/tris.md2");
	grenade->owner = self;
	if(secfunc)
	{
		grenade->touch = Grenade_Explode;
		grenade->think = Grenade_Explode;
		grenade->nextthink = level.time + timer;
	}
	else
	{
		grenade->touch = Grenade_Touch;
		grenade->nextthink = level.time + timer;
		grenade->think = Grenade_Explode;
	}
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "thermal";

	grenade->s.sound = gi.soundindex("weapons/td/tick.wav");

	gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/td/throw.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (grenade);
}

/*
=================
Fire_Sec_Missile
=================
*/

void fire_sec_missile (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

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
//	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->spawnflags |= 1;
	grenade->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + 3;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "thermal";

	grenade->s.sound = gi.soundindex("weapons/td/tick.wav");

	gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/mtube/fire.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (grenade);
}

/*
=================
fire_missile
=================
*/

void missile_fly (edict_t *ent)
{
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 15;
	ent->s.effects |= EF_HYPERBLASTER;
	ent->s.renderfx |=  RF_MINLIGHT;

	VectorScale (ent->movedir, ent->speed, ent->velocity);
}

void missile_anim (edict_t *ent)
{
	if(ent->s.frame == 3)
	{
		missile_fly(ent);
	}
	else
	{
		ent->s.frame++;
		ent->nextthink = level.time + 0.1;
//		ent->s.origin[2] -= 10;
	}
}

void missile_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_MISSILETUBE);
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

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_MISSILETUBE);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

//	SWTC_MuzzleFlash (self, MZ_MISSILETUBE);

	rocket = G_Spawn();
	rocket->speed = speed;
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed/5, rocket->velocity);
	rocket->velocity[2] -= 50;
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->s.frame = 0;
	rocket->owner = self;
	rocket->touch = missile_touch;
	rocket->nextthink = level.time + 0.2;
	rocket->think = missile_anim;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "missile";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_WRISTROCKET);
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

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_WRISTROCKET);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

//	SWTC_MuzzleFlash (self, MZ_WRISTROCKET);
	
	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
//	rocket->s.effects |= /*EF_ROCKET | EF_ANIM_ALLFAST | EF_SPHERETRANS | */EF_IONRIPPER;
	rocket->s.effects |= EF_ROCKET;
//	rocket->s.renderfx |= 0x00080000;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/wrocket/tris.md2");
//	rocket->s.modelindex = gi.modelindex("sprites/typo.sp2");
	rocket->s.frame = 0;
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void fire_homing (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

//	SWTC_MuzzleFlash (self, MZ_WRISTROCKET);
	
	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
//	rocket->s.effects |= /*EF_ROCKET | EF_ANIM_ALLFAST | EF_SPHERETRANS | */EF_IONRIPPER;
	rocket->s.effects |= EF_ROCKET;
//	rocket->s.renderfx |= 0x00080000;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/wrocket/tris.md2");
//	rocket->s.modelindex = gi.modelindex("sprites/typo.sp2");
	rocket->s.frame = 0;
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + .1;
	rocket->think = MMMDonut;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void MMMDonut (edict_t *ent)
{
	edict_t		*target = NULL;
	edict_t		*blip = NULL;
	vec3_t		targetdir, blipdir;
	int			speed;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if(!(blip->svflags & SVF_MONSTER) && !blip->client)
			continue;
		if(blip == ent->owner)
			continue;
		if(!blip->takedamage)
			continue;
		if(blip->health <= 0)
			continue;
		if(!visible(ent, blip))
			continue;
		if(!infront(ent, blip))
			continue;
		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		blipdir[2] += 16;
		if((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
		{
			target = blip;
			VectorCopy(blipdir, targetdir);
		}
	}

	if(target != NULL)
	{
		VectorNormalize(targetdir);
		VectorScale(targetdir, 0.2, targetdir);
		VectorAdd(targetdir, ent->movedir, targetdir);
		VectorNormalize(targetdir);
		VectorCopy(targetdir, ent->movedir);
		vectoangles(targetdir, ent->s.angles);
		speed = VectorLength(ent->velocity);
		VectorScale(targetdir, speed, ent->velocity);
	}

	ent->nextthink = level.time + .1;

}


void bullet_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_WRISTROCKET);
	}

	G_FreeEdict (ent);
}

void fire_sniper_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bullet;
	
	bullet = G_Spawn();
	VectorCopy (start, bullet->s.origin);
	VectorCopy (dir, bullet->movedir);
	vectoangles (dir, bullet->s.angles);
	VectorScale (dir, speed, bullet->velocity);
	bullet->movetype = MOVETYPE_GRAVBULLET;
	bullet->clipmask = MASK_SHOT;
	bullet->solid = SOLID_BBOX;
	bullet->s.effects |= EF_ROCKET;
	VectorClear (bullet->mins);
	VectorClear (bullet->maxs);
	bullet->s.modelindex = gi.modelindex ("models/objects/wrstrkt/tris.md2");
	bullet->owner = self;
	bullet->touch = bullet_touch;
	bullet->nextthink = level.time + 8;
	bullet->think = G_FreeEdict;
	bullet->dmg = damage;
	bullet->classname = "bullet";

	gi.linkentity (bullet);
}
