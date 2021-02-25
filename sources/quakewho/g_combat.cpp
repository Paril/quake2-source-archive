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
// g_combat.c

#include "g_local.h"

/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
bool CanDamage (const edict_t &targ, const edict_t &inflictor)
{
	// bmodels need special checking because their origin is 0,0,0
	if (targ.movetype == MOVETYPE_PUSH)
	{
		const vec3_t dest = (targ.absmin + targ.absmax) * 0.5f;
		const trace_t trace = gi.trace (inflictor.s.origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0f)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}

	static constexpr vec3_t offsets[] = {
		{ 0, 0, 0 },
		{ 15, 15, 0 },
		{ 15, -15, 0 },
		{ -15, 15, 0 },
		{ -15, -15, 0 }
	};
	
	for (auto &offset : offsets)
	{
		trace_t trace = gi.trace (inflictor.s.origin, targ.s.origin + offset, inflictor, MASK_SOLID);

		if (trace.fraction == 1.0f)
			return true;
	}

	return false;
}


/*
============
Killed
============
*/
static void Killed (edict_t &targ, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point)
{
	if (targ.health < -999)
		targ.health = -999;

	targ.enemy = attacker;

	if (targ.movetype == MOVETYPE_PUSH || targ.movetype == MOVETYPE_STOP || targ.movetype == MOVETYPE_NONE)
	{
		// doors, triggers, etc
		targ.die (targ, inflictor, attacker, damage, point);
		return;
	}

	if ((targ.svflags & SVF_MONSTER) && (targ.deadflag != true))
		targ.touch = nullptr;

	targ.die (targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
static void SpawnDamage (const temp_event_t &type, const vec3_t &origin, const vec3_t &normal)
{
	gi.WriteByte (SVC_TEMP_ENTITY);
	gi.WriteByte (type);
	gi.WritePosition (origin);
	gi.WriteDir (normal);
	gi.multicast (origin, MULTICAST_PVS);
}

void T_Damage (edict_t &targ, edict_t &inflictor, edict_t &attacker, const vec3_t &dir, const vec3_t &point, const vec3_t &normal, int32_t damage, int32_t knockback, damageflag_t dflags)
{
	if (!targ.takedamage)
		return;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if (targ != attacker && OnSameTeam (targ, attacker) && dmflags.no_friendly_fire)
		damage = 0;

	gclient_t *client = targ.client;
	temp_event_t te_sparks;

	if (dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	if (targ.flags & FL_NO_KNOCKBACK)
		knockback = 0;

// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if (knockback && (targ.movetype != MOVETYPE_NONE) && (targ.movetype != MOVETYPE_BOUNCE) && (targ.movetype != MOVETYPE_PUSH) && (targ.movetype != MOVETYPE_STOP))
		{
			const vec_t mass = max(50, targ.mass);
			vec3_t kvel;
	
			dir.Normalize(kvel);

			if (targ.client && attacker == targ)
				kvel *= 1600.0f * static_cast<vec_t>(knockback) / mass;	// the rocket jump hack...
			else
				kvel *= 500.0f * static_cast<vec_t>(knockback) / mass;

			targ.velocity += kvel;
		}
	}

	int32_t take = damage;

	// check for godmode
	if ((targ.flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		take = 0;
		SpawnDamage (te_sparks, point, normal);
	}

	if ((targ.svflags & SVF_MONSTER) && (attacker == game.world() || attacker.solid == SOLID_BSP))
	{
		targ.monsterinfo.next_runwalk_check = targ.monsterinfo.should_stand_check = 0;
		targ.ideal_yaw = frandom(360);
		return;
	}

// do the damage
	if (take)
	{
		if ((targ.svflags & SVF_MONSTER) || client)
			SpawnDamage (TE_BLOOD, point, normal);
		else
			SpawnDamage (te_sparks, point, normal);

		targ.health -= take;

		if (take && (targ.svflags & SVF_MONSTER) && !targ.control && attacker.client)
			T_Damage(attacker, attacker, attacker, vec3_origin, vec3_origin, vec3_origin, damage, 0, DAMAGE_NO_PROTECTION);
			
		if (targ.health <= 0)
		{
			if ((targ.svflags & SVF_MONSTER) || client)
				targ.flags |= FL_NO_KNOCKBACK;
			Killed (targ, inflictor, attacker, take, point);
			return;
		}

		if (targ.pain)
		{
			if (targ.svflags & SVF_MONSTER)
			{
				if (targ.health < targ.max_health * 0.5f)
					targ.s.skinnum = targ.monsterinfo.damaged_skin;
				else
					targ.s.skinnum = targ.monsterinfo.undamaged_skin;

				if (prandom(15))
					targ.monsterinfo.next_runwalk_check -= irandom(1, 16);
				if (prandom(15))
					targ.monsterinfo.should_stand_check -= irandom(1, 16);
				if (prandom(5))
					targ.ideal_yaw = frandom(360);
			}
			targ.pain (targ, attacker, knockback, take);
		}
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client)
	{
		client->damage_blood += take;
		client->damage_knockback += knockback;
		client->damage_from = point;
	}
}

/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage (edict_t &inflictor, edict_t &attacker, const vec_t &damage, const edict_ref &ignore, const vec_t &radius)
{
	edict_ref ent = nullptr;

	while ((ent = findradius(ent, inflictor.s.origin, radius)))
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		vec3_t v = ent->mins + ent->maxs;
		v = ent->s.origin + (v * 0.5f);
		v = inflictor.s.origin - v;
		int32_t points = damage - 0.5f * v.Length();
		if (ent == attacker)
			points = points * 0.5f;
		if (points <= 0)
			continue;

		if (!CanDamage (ent, inflictor))
			continue;

		vec3_t dir = ent->s.origin - inflictor.s.origin;
		T_Damage (ent, inflictor, attacker, dir, inflictor.s.origin, vec3_origin, static_cast<int32_t>(points), static_cast<int32_t>(points), DAMAGE_RADIUS);
	}
}
