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

/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &kick, const int32_t &te_impact, const int32_t &hspread, const int32_t &vspread)
{
	bool water = false;
	brushcontents_t	content_mask = MASK_SHOT | MASK_WATER;
	vec3_t water_start;
	trace_t tr = gi.trace (self.s.origin, start, self, MASK_SHOT);

	if (!(tr.fraction < 1.0f))
	{
		vec3_t dir = aimdir.ToAngles();
		auto [ forward, right, up ] = dir.AngleVectors();

		vec_t r = crandom()*hspread;
		vec_t u = crandom()*vspread;
		vec3_t end = start + (forward * 8192) + (right * r) + (up * u);

		if (gi.pointcontents(start) & MASK_WATER)
		{
			water = true;
			water_start = start;
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			water = true;
			water_start = tr.endpos;

			if (start != tr.endpos)
			{
				splashtype_t color = SPLASH_UNKNOWN;

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


				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (SVC_TEMP_ENTITY);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				dir = (end - start).ToAngles();
				dir.AngleVectors(&forward, &right, &up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				end = water_start + (forward * 8192) + (right * r) + (up * u);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (tr.fraction < 1.0f && !(tr.surface && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.ent->takedamage)
			T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET);
		else if (tr.surface && strncmp (tr.surface->name, "sky", 3) != 0)
		{
			gi.WriteByte (SVC_TEMP_ENTITY);
			gi.WriteByte (te_impact);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			gi.multicast (tr.endpos, MULTICAST_PVS);
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t dir = tr.endpos - water_start;
		dir.Normalize();
		
		vec3_t pos = tr.endpos + (dir * -2);

		if (gi.pointcontents (pos) & MASK_WATER)
			tr.endpos = pos;
		else
			tr = gi.trace (pos, water_start, tr.ent, MASK_WATER);

		pos = (water_start + tr.endpos) * 0.5f;

		gi.WriteByte (SVC_TEMP_ENTITY);
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
void fire_bullet (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &kick, const int32_t &hspread, const int32_t &vspread)
{
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &kick, const int32_t &hspread, const int32_t &vspread, const int32_t &count)
{
	for (int32_t i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
static void blaster_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (other == self.owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (other.takedamage)
		T_Damage (other, self, self.owner, self.velocity, self.s.origin, plane->normal, self.dmg, 1, DAMAGE_ENERGY);
	else
	{
		gi.WriteByte (SVC_TEMP_ENTITY);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self.s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self.s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t &self, const vec3_t &start, const vec3_t &dir, const int32_t &damage, const int32_t &speed, const entity_effects_t &effect)
{
	edict_t &bolt = G_Spawn();
	bolt.svflags = SVF_DEADMONSTER;
	// yes, I know it looks weird that projectiles are deadmonsters
	// what this means is that when prediction is used against the object
	// (blaster/hyperblaster shots), the player won't be solid clipped against
	// the object.  Right now trying to run into a firing hyperblaster
	// is very jerky since you are predicted 'against' the shots.
	bolt.s.origin = start;
	bolt.s.old_origin = start;
	bolt.s.angles = dir.ToAngles();
	bolt.velocity = dir * speed;
	bolt.movetype = MOVETYPE_FLYMISSILE;
	bolt.clipmask = MASK_SHOT;
	bolt.solid = SOLID_BBOX;
	bolt.s.effects |= effect;
	bolt.s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt.s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt.owner = self;
	bolt.touch = blaster_touch;
	bolt.nextthink = level.time + 2000;
	bolt.think = G_FreeEdict;
	bolt.dmg = damage;
	bolt.classname = "bolt";
	bolt.Link();

	trace_t tr = gi.trace (self.s.origin, bolt.s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0f)
	{
		bolt.s.origin += dir * -10;
		bolt.touch (bolt, tr.ent, nullptr, nullptr);
	}
}


/*
=================
fire_grenade
=================
*/
static void Grenade_Explode (edict_t &ent)
{
	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent.enemy)
	{
		vec3_t v = ent.enemy->mins + ent.enemy->maxs;
		v = ent.enemy->s.origin + (v * 0.5f);
		v = ent.s.origin - v;
		
		const vec_t points = ent.dmg - 0.5f * v.Length();
		const vec3_t dir = ent.enemy->s.origin - ent.s.origin;
		
		T_Damage (ent.enemy, ent, ent.owner, dir, ent.s.origin, vec3_origin, static_cast<int32_t>(points), static_cast<int32_t>(points), DAMAGE_RADIUS);
	}

	T_RadiusDamage(ent, ent.owner, ent.dmg, ent.enemy, ent.dmg_radius);

	const vec3_t origin = ent.s.origin + (ent.velocity * -0.02f);
	
	gi.WriteByte (SVC_TEMP_ENTITY);
	if (ent.waterlevel)
	{
		if (ent.groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent.groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent.s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t &ent, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (other == ent.owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other.takedamage)
	{
		if (ent.spawnflags & 1)
		{
			if (prandom(50))
				ent.PlaySound(gi.soundindex ("weapons/hgrenb1a.wav"), CHAN_VOICE);
			else
				ent.PlaySound(gi.soundindex ("weapons/hgrenb2a.wav"), CHAN_VOICE);
		}
		else
			ent.PlaySound(gi.soundindex ("weapons/grenlb1b.wav"), CHAN_VOICE);
		return;
	}

	ent.enemy = other;
	Grenade_Explode (ent);
}

void fire_grenade (edict_t &self, const vec3_t &start, const vec3_t &aimdir, const int32_t &damage, const int32_t &speed, const gtime_t &timer, const vec_t &damage_radius)
{
	const auto &[ forward, right, up ] = aimdir.ToAngles().AngleVectors();

	edict_t &grenade = G_Spawn();
	grenade.s.origin = start;
	grenade.velocity = aimdir * speed;
	grenade.velocity += up * (200 + crandom() * 10.0f);
	grenade.velocity += right * (crandom() * 10.0f);
	grenade.avelocity = { 300.f, 300.f, 300.f };
	grenade.movetype = MOVETYPE_BOUNCE;
	grenade.clipmask = MASK_SHOT;
	grenade.solid = SOLID_BBOX;
	grenade.s.effects |= EF_GRENADE;
	grenade.s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade.owner = self;
	grenade.touch = Grenade_Touch;
	grenade.nextthink = level.time + timer;
	grenade.think = Grenade_Explode;
	grenade.dmg = damage;
	grenade.dmg_radius = damage_radius;
	grenade.classname = "grenade";
	grenade.Link();
}
