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
// g_weapon.c

#include "g_local.h"
#include "m_player.h"

constexpr vec3_t P_ProjectSource (const gclient_t *client, const vec3_t &point, const vec3_t &distance, const vec3_t &forward, const vec3_t &right, const vec3_t &up)
{
	vec3_t _distance = distance;
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	return G_ProjectSource (point, _distance, forward, right, up);
}

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t &ent)
{
	ent.client->pers.lastweapon = ent.client->pers.weapon;
	ent.client->pers.weapon = ent.client->newweapon;
	ent.client->newweapon = nullptr;

	// set visible model
	if (ent.s.modelindex == MODEL_PLAYER)
	{
		int32_t i;

		if (ent.client->pers.weapon)
			i = ((ent.client->pers.weapon->weapmodel + 1) & 0xff) << 8;
		else
			i = 0;

		ent.s.skinnum = (ent.s.number - 1) | i;
	}

	if (!ent.client->pers.weapon)
	{
		// dead
		ent.client->ps.gunindex = MODEL_NONE;
		return;
	}

	ent.client->weaponstate = WEAPON_ACTIVATING;
	ent.client->ps.gunframe = 0;
	ent.client->ps.gunindex = gi.modelindex(ent.client->pers.weapon->view_model);

	ent.client->anim_priority = ANIM_PAIN;

	if (ent.client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent.s.frame = FRAME_crpain1;
		ent.client->anim_end = FRAME_crpain4;
	}
	else
	{
		ent.s.frame = FRAME_pain301;
		ent.client->anim_end = FRAME_pain304;
	}
}

/*
=================
NoAmmoWeaponChange
=================
*/
static void NoAmmoWeaponChange (edict_t &ent)
{
	if (level.time >= ent.pain_debounce_time)
	{
		ent.PlaySound(gi.soundindex("weapons/noammo.wav"), CHAN_WEAPON);
		ent.pain_debounce_time = level.time + 1000;
	}

	ent.client->newweapon = &g_weapons[WEAP_SHOTGUN];

	/*if (ent.client->pers.ammo[AMMO_BULLETS])
		ent.client->newweapon = &g_weapons[WEAP_MACHINEGUN];
	else*/ if (ent.client->pers.ammo[AMMO_SHELLS])
		ent.client->newweapon = &g_weapons[WEAP_SHOTGUN];
	else if (ent.client->pers.ammo[AMMO_GRENADES])
		ent.client->newweapon = &g_weapons[WEAP_GRENADELAUNCHER];
}

void EnsureGoodPosition(edict_t &whomst)
{
	trace_t tr = gi.trace(whomst.s.origin, whomst.mins, whomst.maxs, whomst.s.origin, whomst, MASK_PLAYERSOLID);

	// all good!
	if (!tr.allsolid && !tr.startsolid && tr.fraction == 1.0f)
		return;
	
	for (int32_t x = -64; x <= 64; x += 8)
	for (int32_t y = -64; y <= 64; y += 8)
	{
		const vec3_t spot = whomst.s.origin + vec3_t { static_cast<vec_t>(x), static_cast<vec_t>(y), 0 };

		tr = gi.trace(spot, whomst.mins, whomst.maxs, spot, whomst, MASK_PLAYERSOLID);

		// all good!
		if (!tr.allsolid && !tr.startsolid && tr.fraction == 1.0f)
		{
			whomst.s.origin = tr.endpos;
			return;
		}
	}

	gi.dprintf("Couldn't find a good spot :(");
}

void Possess(edict_t &player, edict_t &monster, const bool &clone)
{
	player.movetype = MOVETYPE_NOCLIP;
	player.solid = SOLID_NOT;
	player.svflags |= SVF_NOCLIENT;
	player.client->regen_debounce = level.time + 5000;
	player.Link();

	edict_t &copy = clone ? G_Spawn() : monster;

	if (clone)
	{
		const int32_t number = copy.s.number;
		monster.Unlink();
		copy = monster;
		copy.s.number = number;
		player.Link();
		monster.Link();
	}

	copy.monsterinfo.idle_time = 0;
	copy.health = player.health;

	if (clone)
	{
		copy.s.origin = player.s.origin;
		copy.s.origin[2] += player.mins[2];
		copy.s.origin[2] -= copy.mins[2];
		EnsureGoodPosition(copy);
		copy.Link();
	}

	copy.control = player;
	player.control = copy;

	player.client->cmd.upmove = 0;
}

static void Shift(edict_t &control, edict_t &other)
{
	// shift these bodies; "control" will become a regular map monster (and die), and "other" will become the player's monster
	for (auto it = level.monsters.begin(); it != level.monsters.end(); it++)
	{
		if (*it == other)
		{
			// found it! remove from the monsters pool
			level.monsters.erase(it);
			break;
		}
	}

	// add the control monster to the pool
	level.monsters.push_back(control);

	// swap controller
	edict_t &player = control.control;

	player.health = player.control->health;
	other.health = player.health;

	player.control = other;
	other.control = player;

	control.control = nullptr;

	// kill the monster
	control.die(control, control, control, control.health, vec3_origin);
}

static void Think_HidersWeapon(edict_t &ent)
{
	for (auto &other : game.players)
	{
		if (ent == other || !other.client || other.client->resp.spectator || other.client->resp.team != ent.client->resp.team ||
			!gi.inPVS(ent.s.origin, other.s.origin))
			continue;

		gi.WriteByte(SVC_TEMP_ENTITY);
		gi.WriteByte(TE_GREENBLOOD);
		gi.WritePosition(other.s.origin + vec3_t { 0, 0, other.maxs[2] + 12 });
		gi.WriteDir({ 0, 0, 1 });
		ent.Unicast(true);
	}

	if (ent.control)
	{
		if (ent.client->regen_debounce < level.time)
		{
			auto half = ent.control->max_health * 0.5f;

			if (ent.control->health < half)
			{
				ent.control->health++;

				if (ent.control->health >= half)
					ent.control->s.skinnum = ent.control->monsterinfo.undamaged_skin;
			}

			ent.client->regen_debounce = level.time + 1000;
		}

		if (ent.client->num_jumps && level.control_delay < level.time)
		{
			edict_ref jump_target = nullptr;
			const vec3_t start = ent.s.origin + vec3_t{ 0, 0, ent.viewheight };
			const vec3_t forward = ent.client->ps.viewangles.Forward();
			const vec3_t end = start + (forward * 8192);

			const trace_t tr = gi.trace(start, end, ent.control, CONTENTS_MONSTER | MASK_SOLID);

			if (tr.fraction < 1.0f && (tr.ent->svflags & SVF_MONSTER) && !tr.ent->control)
				jump_target = tr.ent;

			if (jump_target)
			{
				if (ent.client->latched_buttons & BUTTON_ATTACK)
				{
					Shift(ent.control, jump_target);
					ent.client->latched_buttons &= ~BUTTON_ATTACK;
					ent.client->num_jumps--;
				}
				else
				{
					gi.WriteByte(SVC_TEMP_ENTITY);
					gi.WriteByte(TE_BFG_LASER);
					gi.WritePosition(jump_target->s.origin + vec3_t { 0, 0, jump_target->maxs[2] });
					gi.WritePosition(jump_target->s.origin + vec3_t { 0, 0, jump_target->maxs[2] + 24 });
					ent.Unicast(true);
				}
			}
		}

		return;
	}

	if ((ent.client->buttons | ent.client->latched_buttons) & BUTTON_ATTACK)
	{
		ent.client->latched_buttons &= ~BUTTON_ATTACK;

		const vec3_t start = ent.s.origin + vec3_t{ 0, 0, ent.viewheight };
		const vec3_t forward = ent.client->ps.viewangles.Forward();
		const vec3_t end = start + (forward * 64);

		trace_t tr = gi.trace(start, end, ent, CONTENTS_MONSTER | MASK_SOLID);

		if (tr.fraction == 1.0f || !(tr.ent->svflags & SVF_MONSTER))
			return;

		Possess(ent, tr.ent, true);
	}
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t &ent)
{
	if (ent.client->resp.spectator)
		return;

	// hiders have no weapons
	if (ent.client->resp.team == TEAM_HIDERS)
	{
		Think_HidersWeapon(ent);
		return;
	}

	// if just died, put the weapon away
	if (ent.health < 1)
	{
		ent.client->newweapon = nullptr;
		ChangeWeapon (ent);
	}

	// call active weapon think routine
	if (ent.client->pers.weapon && ent.client->pers.weapon->weaponthink)
		ent.client->pers.weapon->weaponthink (ent);
}

/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t &ent, gitem_t *item)
{
	if (!ent.client->resp.team)
	{
		if (!ent.client->resp.ready && item == &g_weapons[WEAP_BLASTER])
		{
			ent.client->resp.ready = true;
			ent.client->Print("You've readied up!\n");
			G_CheckPlayerReady();
		}
		return;
	}

	// see if we're already using it
	if (ent.client->pers.weapon == item)
		return;

	if (item->ammo != AMMO_NONE && !g_select_empty->value)
	{
		if (!ent.client->pers.ammo[item->ammo])
		{
			ent.client->Print("No ammo for %s.\n", item->pickup_name);
			return;
		}

		if (ent.client->pers.ammo[item->ammo] < ent.client->pers.weapon->quantity)
		{
			ent.client->Print("Not enough ammo for %s.\n", item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent.client->newweapon = item;
}

/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
static void Weapon_Generic (edict_t &ent, const int32_t &FRAME_ACTIVATE_LAST, const int32_t &FRAME_FIRE_LAST, const int32_t &FRAME_IDLE_LAST, const int32_t &FRAME_DEACTIVATE_LAST,
	const std::initializer_list<int32_t> &pause_frames, const std::initializer_list<int32_t> &fire_frames, void (*fire)(edict_t &ent))
{
	const int32_t FRAME_FIRE_FIRST		= (FRAME_ACTIVATE_LAST + 1);
	const int32_t FRAME_IDLE_FIRST		= (FRAME_FIRE_LAST + 1);
	const int32_t FRAME_DEACTIVATE_FIRST= (FRAME_IDLE_LAST + 1);

	if(ent.deadflag || ent.s.modelindex != MODEL_PLAYER) // VWep animations screw up corpses
		return;

	if (ent.client->weaponstate == WEAPON_DROPPING)
	{
		if (ent.client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent.client->ps.gunframe) == 4)
		{
			ent.client->anim_priority = ANIM_REVERSE;
			
			if(ent.client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent.s.frame = FRAME_crpain4+1;
				ent.client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent.s.frame = FRAME_pain304+1;
				ent.client->anim_end = FRAME_pain301;
			}
		}

		ent.client->ps.gunframe = min(ent.client->ps.gunframe + 2, FRAME_DEACTIVATE_LAST);
		return;
	}

	if (ent.client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent.client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent.client->weaponstate = WEAPON_READY;
			ent.client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}
		
		ent.client->ps.gunframe = min(ent.client->ps.gunframe + 2, FRAME_ACTIVATE_LAST);
		return;
	}

	if (ent.client->newweapon && ent.client->weaponstate != WEAPON_FIRING)
	{
		ent.client->weaponstate = WEAPON_DROPPING;
		ent.client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent.client->anim_priority = ANIM_REVERSE;
			
			if(ent.client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent.s.frame = FRAME_crpain4+1;
				ent.client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent.s.frame = FRAME_pain304+1;
				ent.client->anim_end = FRAME_pain301;
			}
		}

		return;
	}

	if (ent.client->weaponstate == WEAPON_READY)
	{
		if (((ent.client->latched_buttons | ent.client->buttons) & BUTTON_ATTACK))
		{
			ent.client->latched_buttons &= ~BUTTON_ATTACK;

			if (ent.client->pers.weapon->ammo == AMMO_NONE || 
				(ent.client->pers.ammo[ent.client->pers.weapon->ammo] >= ent.client->pers.weapon->quantity))
			{
				ent.client->ps.gunframe = FRAME_FIRE_FIRST;
				ent.client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent.client->anim_priority = ANIM_ATTACK;
				if (ent.client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent.s.frame = FRAME_crattak1-1;
					ent.client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent.s.frame = FRAME_attack1-1;
					ent.client->anim_end = FRAME_attack8;
				}
			}
			else
				NoAmmoWeaponChange (ent);
		}
		else
		{
			if (ent.client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent.client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			for (auto &n : pause_frames)
				if (ent.client->ps.gunframe == n && prandom(94))
					return;

			ent.client->ps.gunframe++;
			return;
		}
	}

	if (ent.client->weaponstate == WEAPON_FIRING)
	{
		bool fired = false;

		for (auto &n : fire_frames)
		{
			if (ent.client->ps.gunframe == n)
			{
				fire (ent);
				fired = true;
				break;
			}
		}

		if (!fired)
			ent.client->ps.gunframe++;

		if (ent.client->ps.gunframe == FRAME_IDLE_FIRST + 1)
			ent.client->weaponstate = WEAPON_READY;
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/
static void weapon_grenadelauncher_fire (edict_t &ent)
{
	constexpr int32_t damage = 120;
	constexpr vec_t radius = damage + 40;
	const vec3_t offset { 8.f, 8.f, ent.viewheight - 8.f };
	const auto &[ forward, right, up ] = ent.client->v_angle.AngleVectors();
	const vec3_t start = P_ProjectSource (ent.client, ent.s.origin, offset, forward, right, up);

	ent.client->kick_origin = forward * -2;
	ent.client->kick_angles[0] = -1;

	fire_grenade (ent, start, forward, damage, 600, 2500, radius);

	gi.WriteByte (SVC_MUZZLEFLASH);
	gi.WriteEntity (ent);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (ent.s.origin, MULTICAST_PVS);

	ent.client->ps.gunframe++;

	if (!dmflags.infinite_ammo)
		ent.client->pers.ammo[ent.client->pers.weapon->ammo]--;
}

void Weapon_GrenadeLauncher (edict_t &ent)
{
	Weapon_Generic (ent, 5, 16, 59, 64, { 34, 51, 59 }, { 6 }, weapon_grenadelauncher_fire);
}

/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/
static void Blaster_Fire (edict_t &ent, const vec3_t &g_offset, const int32_t &damage, const entity_effects_t &effect)
{
	const auto &[ forward, right, up ] = ent.client->v_angle.AngleVectors();
	const vec3_t offset = vec3_t { 24.f, 8.f, ent.viewheight - 8.f } + g_offset;
	const vec3_t start = P_ProjectSource (ent.client, ent.s.origin, offset, forward, right, up);

	ent.client->kick_origin = forward * -2;
	ent.client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect);

	// send muzzle flash
	gi.WriteByte (SVC_MUZZLEFLASH);
	gi.WriteEntity (ent);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent.s.origin, MULTICAST_PVS);
}

static void Weapon_Blaster_Fire (edict_t &ent)
{
	constexpr int32_t damage = 15;
	Blaster_Fire (ent, vec3_origin, damage, EF_BLASTER);
	ent.client->ps.gunframe++;
}

void Weapon_Blaster (edict_t &ent)
{
	Weapon_Generic (ent, 4, 8, 52, 55, { 19, 32 }, { 5 }, Weapon_Blaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/
static void Machinegun_Fire (edict_t &ent)
{
	constexpr int32_t damage = 5;
	constexpr int32_t kick = 2;

	if (!(ent.client->buttons & BUTTON_ATTACK))
	{
		ent.client->ps.gunframe++;
		return;
	}

	if (ent.client->ps.gunframe == 5)
		ent.client->ps.gunframe = 4;
	else
		ent.client->ps.gunframe = 5;

	if (ent.client->pers.ammo[ent.client->pers.weapon->ammo] < 1)
	{
		ent.client->ps.gunframe = 6;
		NoAmmoWeaponChange (ent);
		return;
	}

	for (size_t i = 1; i < 3; i++)
	{
		ent.client->kick_origin[i] = crandom() * 0.35f;
		ent.client->kick_angles[i] = crandom() * 0.7f;
	}

	ent.client->kick_origin[0] = crandom() * 0.35f;

	// get start / end positions
	const vec3_t angles = ent.client->v_angle + ent.client->kick_angles;
	const auto &[ forward, right, up ] = angles.AngleVectors();
	const vec3_t offset { 0.f, 8.f, ent.viewheight - 8.f };
	const vec3_t start = P_ProjectSource (ent.client, ent.s.origin, offset, forward, right, up);

	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD / 3, DEFAULT_BULLET_HSPREAD / 3);

	gi.WriteByte (SVC_MUZZLEFLASH);
	gi.WriteEntity (ent);
	gi.WriteByte (MZ_MACHINEGUN);
	gi.multicast (ent.s.origin, MULTICAST_PVS);

	if (!dmflags.infinite_ammo)
		ent.client->pers.ammo[ent.client->pers.weapon->ammo]--;

	ent.client->anim_priority = ANIM_ATTACK;
	if (ent.client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent.s.frame = FRAME_crattak1 - prandom(25);
		ent.client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent.s.frame = FRAME_attack1 - prandom(25);
		ent.client->anim_end = FRAME_attack8;
	}
}

void Weapon_Machinegun (edict_t &ent)
{
	Weapon_Generic (ent, 3, 5, 45, 49, { 23, 45 }, { 4, 5 }, Machinegun_Fire);
}

/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/
static void weapon_shotgun_fire (edict_t &ent)
{
	constexpr int32_t damage = 6;
	constexpr int32_t kick = 8;

	if (ent.client->ps.gunframe == 9)
	{
		ent.client->ps.gunframe++;
		return;
	}

	const auto &[ forward, right, up ] = ent.client->v_angle.AngleVectors();

	ent.client->kick_origin = forward * -2;
	ent.client->kick_angles[0] = -2;

	const vec3_t offset { 0.f, 8.f, ent.viewheight - 8.f };
	const vec3_t start = P_ProjectSource (ent.client, ent.s.origin, offset, forward, right, up);
	fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT);

	// send muzzle flash
	gi.WriteByte (SVC_MUZZLEFLASH);
	gi.WriteEntity (ent);
	gi.WriteByte (MZ_SHOTGUN);
	gi.multicast (ent.s.origin, MULTICAST_PVS);

	ent.client->ps.gunframe++;

	if (!dmflags.infinite_ammo)
		ent.client->pers.ammo[ent.client->pers.weapon->ammo]--;
}

void Weapon_Shotgun (edict_t &ent)
{
	Weapon_Generic (ent, 7, 18, 36, 39, { 22, 28, 34 }, { 8, 9 }, weapon_shotgun_fire);
}
