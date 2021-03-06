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


static qboolean	is_quad;
static byte		is_silenced;


void weapon_grenade_fire (edict_t *ent, qboolean held);


static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;

	index = ITEM_INDEX(ent->item);

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{

		//peewee start
		//dont give ammo with beamsword
		if (ent->item != FindItem("Beam Sword"))
		{
			// give them some ammo with it
			ammo = FindItem (ent->item->ammo);
			if ( (int)dmflags->value & DF_INFINITE_AMMO )
				Add_Ammo (other, ammo, 1000);
			else
				Add_Ammo (other, ammo, ammo->quantity);
		}
		//peewee end

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

	//peewee start
	//removed so that it doesnt switch weapons while still firing plasma
	//if (other->client->pers.weapon != ent->item &&
	//	(other->client->pers.inventory[index] == 1) &&
	//	( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
	//	other->client->newweapon = ent->item;
	//peewee end

	return true;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
	int i;

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	//peewee start
	//beam sword does not use ammo
	if (ent->client->pers.weapon == FindItem("Beam Sword"))
	{
		ent->client->ammo_index = 0;
	}
	else if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;
	//peewee end

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

	//peewee start
	//Tank does not have weapon model
	//if (!ent->client->pers.enhance == ENH_RTANK)
	//	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
	//peewee end


	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
			ent->s.frame = FRAME_pain301;
			ent->client->anim_end = FRAME_pain304;

	}
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
	//peewee start
	//if we still have shots out, do not change weapon
	if (ent->client->pers.shot_limit > 0)
		return;
	//peewee end

	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Blue Energy"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Gemini Laser"))] )
	{
		ent->client->newweapon = FindItem ("Gemini Laser");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Yellow Energy"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Needle Cannon"))] )
	{
		ent->client->newweapon = FindItem ("Needle Cannon");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Gray Energy"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Rolling Cutter"))] )
	{
		ent->client->newweapon = FindItem ("Rolling Cutter");
		return;
	}
//peewee start
//	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
//		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Green Energy"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Crash Bomb"))] )
//peewee end
	{
		ent->client->newweapon = FindItem ("Crash Bomb");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("White Energy"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Skull Shield"))] )
	{
		ent->client->newweapon = FindItem ("Skull Shield");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("White Energy"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Dust Crusher"))] )
	{
		ent->client->newweapon = FindItem ("Dust Crusher");
		return;
	}
	ent->client->newweapon = FindItem ("blaster");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	//peewee start
	//do not use a new weapon while shots still exist for current
	if (ent->client->pers.shot_limit > 0)
		return;
	//peewee end


	//peewee start
	//beam sword does not use ammo
	if (item != FindItem("Beam Sword"))
	{
		if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
		{
			ammo_item = FindItem(item->ammo);
			ammo_index = ITEM_INDEX(ammo_item);

			if (!ent->client->pers.inventory[ammo_index])
			{
				gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
				return;
			}

			if (ent->client->pers.inventory[ammo_index] < item->quantity)
			{
				gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
				return;
			}
		}
	}
	//peewee end

	// change to this weapon when down
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;

			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;

			}
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) ||
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
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

	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
//	fire_magnet (ent, start, forward, damage, 600, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster (ent, start, forward, damage, 1000, effect, hyper);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	if (hyper)
		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
	else
		gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}


/*void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}*/

//peewee start
void Weapon_Blaster_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	int		damage;
	int		type;

	//Can only fire 3 at a time
	if (ent->client->pers.shot_limit > 2)
	{
		ent->client->ps.gunframe = 8;
		return;
	}

	//if button is held
	if ((ent->client->buttons) & BUTTON_ATTACK)
	{
		//if already started charging
		if (ent->client->pers.charge > 0.4 &&
			ent->client->pers.charge < 1 &&
			ent->client->pers.chargesnd < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/charge1.wav"), 1, ATTN_NORM, 0);
			ent->client->pers.chargesnd = level.time + 0.64;
		}
		//if we charged all the way
		else if (ent->client->pers.charge > 1 &&
				 ent->client->pers.charge < 2 &&
				 ent->client->pers.chargesnd < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/charge2.wav"), 1, ATTN_NORM, 0);
			ent->client->pers.chargesnd = level.time + 0.74;
		}
		//if we are still charging and chargenum is divisible by 2
		else if (ent->client->pers.charge > 2 &&
				ent->client->pers.chargesnd < level.time)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/charge3.wav"), 1, ATTN_NORM, 0);
			ent->client->pers.chargesnd = level.time + 0.29;
		}

		ent->client->pers.charge += 0.2;

		return;
	}


	type = (int) ent->client->pers.charge;

	if (type == 0)
		damage = 15;
	else if (type == 1)
		damage = 25;
	else
		damage = 40;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_cblaster (ent, start, forward, damage, 700, type, true);

	//increment shots
	ent->client->pers.shot_limit++;

	//clear charge
	ent->client->pers.charge = 0;

	//make custom noise
	//gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/bust.wav"), 1, ATTN_NORM, 0);


	//skip ahead to speed up fire frames
	ent->client->ps.gunframe = 8;
}
//charge up blaster


//peewee end

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	//static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 15;
			else
				damage = 20;
			Blaster_Fire (ent, offset, damage, true, effect);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
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

		PlayerNoise(ent, start, PNOISE_WEAPON);
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================




//peewee start
//secondary charge up blaster functionality
//peewee start
void Fire_Blast2 (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	int		damage;
	int		type;

	//set charge to atleast 0.1
	if (ent->client->pers.charge_2 == 0)
		ent->client->pers.charge_2 = 0.01;

	//if use button is held then blast time will be a negative number
	if (ent->client->pers.blast_time == -1)
	{
		//increment charge2
		//ent->client->pers.charge_2 += 0.022;
		ent->client->pers.charge_2 += 0.07;

		//gi.cprintf (ent, PRINT_HIGH, "Charging...\n");
		return;
	}

    //gi.cprintf (ent, PRINT_HIGH, "Fire!\n");

	type = (int) ent->client->pers.charge_2;

	if (type == 0)
		damage = 15;
	else if (type == 1)
		damage = 25;
	else
		damage = 40;

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	//if we're shooting the biggest type
	if (damage == 40)
		//move the blast to the side more to avoid collisions
		VectorSet(offset, 16, -16, ent->viewheight-8);
	else
		VectorSet(offset, 16, -8, ent->viewheight-8);

	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_cblaster (ent, start, forward, damage, 650, type, false);

	//clear charge
	ent->client->pers.charge_2 = 0;

	//set reload time
	ent->client->pers.blast_time = level.time + 0.5;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}












//peewee start
//fire wallkick
qboolean fire_wallk (edict_t *ent)
{
	vec3_t		start, from, end;
	vec3_t		forward, right;
	vec3_t		offset;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-32);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);


	VectorMA (start, 32, forward, end);
	VectorCopy (start, from);

	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;

	tr = gi.trace (from, NULL, NULL, end, ent, mask);

	//if we hit a wall but not the sky and
	//tr.fraction???? and
	//it wasn't a takedamage and
	//it wasn't the sky

	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)) &&
		tr.fraction < 1.0 &&
		!tr.ent->takedamage &&
		strncmp (tr.surface->name, "sky", 3) != 0)
	{
		//play sound
		gi.sound(ent, CHAN_AUTO, gi.soundindex("items/wallk.wav"), 1, ATTN_NORM, 0);

		//true
		return true;

	}
	else
		return false; //we did not hit a wall, do nothing
}
//peewee end





//peewee start
//fire tank blaster
void Fire_tankblast (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	int		damage;
	int		type;

	damage = 40;

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 16, -16, ent->viewheight-8);

	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_cblaster (ent, start, forward, damage, 700, 2, false);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}


void Fire_tankrock (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	int		damage;
	int		type;

	damage = 50;

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorSet(offset, 8, 16, ent->viewheight + 16);

	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_rocket (ent, start, forward, damage, 650, 70, 70);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}





void Weapon_Cutter_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	//Can only fire 3 at a time
	if (ent->client->pers.shot_limit > 2)
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	//otherwise increment limit
	ent->client->pers.shot_limit++;

	int		damage = 40;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_cutter (ent, start, forward, damage, 700);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Cutter (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Cutter_Fire);
}






void Weapon_Bomb_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	//Can only fire 3 at a time
	if (ent->client->pers.shot_limit > 2)
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	//otherwise increment limit
	ent->client->pers.shot_limit++;

	int		damage = 100;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_bomb(ent, start, forward, damage, 600, 2.0, 100);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/bust.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Bomb (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Bomb_Fire);
}


void Weapon_Needle_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	int		damage = 15;
	int		speed = 1650;


	//disperse quick enough to not need a limit,
	//however we will increment shot_limit so weapon cannot
	//be switched while needles still exist

	//if we are holding in fire button
	if ((ent->client->buttons) & BUTTON_ATTACK)
	{
		//and we at least have 2 bullets and already spent 1 frame shooting only 1 shot
		if (ent->client->pers.inventory[ent->client->ammo_index] > 1 &&
			ent->client->machinegun_shots > 0)
		{
			//mark for 2 shots
			ent->client->machinegun_shots = 2;
		}
		else //otherwise mark for 1 shot
			ent->client->machinegun_shots = 1;
	}
	else //otherwise we're always 1
		ent->client->machinegun_shots = 0;


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;


	//make sure we have ammo before we shoot... keeps subtracting ammo for some aweful reason
	if (ent->client->pers.inventory[ent->client->ammo_index] > 0)
	{
		//fire the normal shot
		fire_needle (ent, start, forward, damage, speed);

		//increment shotlimit
		ent->client->pers.shot_limit++;

		//subtract 1 ammo for normal shot
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/bust.wav"), 1, ATTN_NORM, 0);


	//if we are marked for 2 shots
	if (ent->client->machinegun_shots == 2 &&
		ent->client->pers.inventory[ent->client->ammo_index] > 0)
	{
		vec3_t	v;
		int	u, r;

		//get random up
		u = random() * 6;
		u -= 3;

		//get random right
		r = random() * 6;
		r -= 3;

		//determin new forward
		v[PITCH] = ent->client->v_angle[PITCH] + u;
		v[YAW]   = ent->client->v_angle[YAW] + r;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, NULL, NULL);

		//fire second shot
		fire_needle (ent, start, forward, damage, speed);

		//increment shotlimit
		ent->client->pers.shot_limit++;

		//subtract ammo for second shot
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;

		//if we are on machinegun_shots of 2 then the wind up is finished
		//so skip frames to keep a quick fire rate
		ent->client->ps.gunframe--; //so go back a frame and keep firing
	}
	else //otherwise finish attack animation
	{
		ent->client->ps.gunframe++;

		//if we are not holding button then reset machingun shots
		if (!((ent->client->buttons) & BUTTON_ATTACK))
			ent->client->machinegun_shots = 0;
	}
}

void Weapon_Needle (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Needle_Fire);
}




void Weapon_Crashb_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	//Can only fire 3 at a time
	if (ent->client->pers.shot_limit > 2)
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	//otherwise increment limit
	ent->client->pers.shot_limit++;

	int		damage = 40;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

    fire_crashb (ent, start, forward, damage, 900, 100);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/bust.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Crashb (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Crashb_Fire);
}


//----------------
//Dust Crusher
//----------------
void Weapon_Dust_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	int		damage = 45;

	//Can only fire 1 at a time
	if (ent->client->pers.shot_limit > 0)
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	//otherwise increment limit
	ent->client->pers.shot_limit++;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_dust (ent, start, forward, damage, 700);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/bust.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Dust (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Dust_Fire);
}




void Weapon_Gemini_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	//Can only fire 1 at a time
	if (ent->client->pers.shot_limit > 0)
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	//otherwise increment limit
	ent->client->pers.shot_limit++;

	int		damage = 70;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_gemini (ent, start, forward, damage, 1000);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/gemini.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Gemini (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Gemini_Fire);
}



void Weapon_Magnet_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	//Can only fire 1 at a time
	if (ent->client->pers.shot_limit > 0)
	{
		ent->client->ps.gunframe = 8;
		return;
	}
	//otherwise increment limit
	ent->client->pers.shot_limit++;

	int		damage = 70;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 16, 8, ent->viewheight-8);
	VectorAdd (offset, vec3_origin, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_magnet (ent, start, forward, damage, 550, 70, 100);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/magnet.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->ps.gunframe++;
}

void Weapon_Magnet (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Magnet_Fire);
}


//---------------
//Skull Shield
//---------------
void Weapon_Skulls_Fire (edict_t *ent)
{
	//Can only fire 3 at a time
	if (ent->client->pers.shot_limit > 8)
	{
		ent->client->ps.gunframe = 8;
		return;
	}


	int		damage = 15;

	ent->client->kick_angles[0] = -1;

	fire_skulls (ent, damage, 0);
	fire_skulls (ent, damage, 1);
	fire_skulls (ent, damage, 2);
	fire_skulls (ent, damage, 3);


	//increment shot limit for each skull fired
	ent->client->pers.shot_limit += 4;

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_BLASTER | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/bust.wav"), 1, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]-= 2;

	ent->client->ps.gunframe++;
}

void Weapon_Skulls (edict_t *ent)
{
	static int	pause_frames[]	= {19, 19, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Skulls_Fire);
}



void Weapon_Sabre_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	int		damage = 40;

	if (deathmatch->value)
		damage = 15;


	//Can only fire 1 at a time
	if (ent->client->pers.shot_limit > 0)
	{
		ent->client->ps.gunframe = 12;
		return;
	}

	//charge up to fire blasts from sabre
	//if button is held
	if ((ent->client->buttons) & BUTTON_ATTACK)
	{
		ent->client->pers.charge += 0.2;
		return;
	}

	//otherwise increment limit
	ent->client->pers.shot_limit++;

	//if not charged enough swing normal
	if (ent->client->pers.charge < 2)
	{
		fire_sabre (ent, damage, 0);
	}
	//otherwise we are charged so shoot a blast
	else
	{
		damage = 90;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 12, 8, ent->viewheight-8);
		VectorAdd (offset, vec3_origin, offset);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;

		fire_cblaster (ent, start, forward, damage, 900, -1, true);
	}

	//play sound
	gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/saber1.wav"), 1, ATTN_NORM, 0);

	ent->client->pers.charge = 0;
	ent->client->ps.gunframe++;
}

void Weapon_Sabre (edict_t *ent)
{
	static int	pause_frames[]	= {18, 18, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 12, 46, 51, pause_frames, fire_frames, Weapon_Sabre_Fire);
}
