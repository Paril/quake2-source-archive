// g_weapon.c

#include "g_local.h"
#include "m_player.h"

void MakePlayer (edict_t *);	// SH
void MV (edict_t *);			// SH
void weapon_grenade_fire (edict_t *, qboolean);

static qboolean	is_quad;
static byte		is_silenced;

// SH : used to be type "static void"
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}

// SH
void BoundAmmo (edict_t * ent)
{
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] < 0)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] = 0;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] > ent->client->pers.max_cells)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))] = ent->client->pers.max_cells;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] < 0)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] = 0;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] > ent->client->pers.max_cells)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("slugs"))] = ent->client->pers.max_cells;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("grenades"))] < 0)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("grenades"))] = 0;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("grenades"))] > ent->client->pers.max_grenades)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("grenades"))] = ent->client->pers.max_grenades;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("rockets"))] < 0)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("rockets"))] = 0;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("rockets"))] > ent->client->pers.max_rockets)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("rockets"))] = ent->client->pers.max_rockets;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("shells"))] < 0)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("shells"))] = 0;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("shells"))] > ent->client->pers.max_shells)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("shells"))] = ent->client->pers.max_shells;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("bullets"))] < 0)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("bullets"))] = 0;
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("bullets"))] > ent->client->pers.max_bullets)
		ent->client->pers.inventory[ITEM_INDEX(FindItem ("bullets"))] = ent->client->pers.max_bullets;

//	if (ent->client->pers.inventory[ArmorIndex(ent)] > 200 && ent->client->pers.passive != ARMOR)
//		ent->client->pers.inventory[ArmorIndex(ent)] = 200;
}
// \SH
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

// SH
	if (ent->flags & FL_OBSERVER)
		return false;
// \SH

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
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

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

	if (other->client->pers.weapon != ent->item && 
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
		other->client->newweapon = ent->item;

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

	if (ent->client->grenade_time)
	{
//		ent->client->grenade_time = level.time; SH
//		ent->client->weapon_sound = 0;
//		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;

	if (!(ent->flags & FL_OBSERVER))
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{
// SH
	if (ent->client->pers.special == BOPGUN || ent->client->pers.combo == C_CRIP || ent->client->pers.combo == C_ROBOT || ent->client->pers.combo == C_MAGE)
		return;
// \SH
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
	{
		ent->client->newweapon = FindItem ("railgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
	{
		ent->client->newweapon = FindItem ("hyperblaster");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
	{
		ent->client->newweapon = FindItem ("chaingun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
	{
		ent->client->newweapon = FindItem ("machinegun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
	{
		ent->client->newweapon = FindItem ("super shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))] )
	{
		ent->client->newweapon = FindItem ("shotgun");
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

// SH
	if (ent->client->pers.combo == C_JEDI && !(ent->flags & FL_OBSERVER))
	{
		if (ent->jedi_time < level.time && ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) && ent->health > 0)
		{
			ent->flags |= FL_OPTIC;
			ent->jedi_time = level.time + 0.6;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->weaponstate = WEAPON_FIRING;

		}
		
		return;
	}
// \SH

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots || ent->client->pers.passive == ASS)
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

// SH
	if (ent->client->pers.passive == ROBOT || ent->flags & FL_BERSERK || ent->client->pers.passive == CRIP || ent->client->pers.passive == MAGE || ent->client->pers.special == BOPGUN || ent->client->pers.combo == C_JEDI)
		return;
// \SH

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

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

	if (ent->client->pers.passive == ROBOT || ent->flags & FL_BERSERK || ent->client->pers.passive == CRIP || ent->client->pers.passive == MAGE || ent->client->pers.special == BOPGUN || ent->client->pers.combo == C_JEDI)
		return;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
//		Drop_Item (ent, item);
//		ent->client->pers.inventory[index]--;
//		NoAmmoWeaponChange (ent);

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

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST || FASTSWITCH)  // SH added FASTSWITCH
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
// SH
		if (FASTSWITCH)
			ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST;
		else
// \SH
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

//		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;  SH
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) || ent->ants > level.time) // SH
		{
// SH
			if (ent->flags & FL_OBSERVER)
			{
				ent->client->latched_buttons &= ~BUTTON_ATTACK;
				MakePlayer(ent);
				return;
			}
// \SH
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity)
				 || ent->client->pers.special == BOPGUN) // SH
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED && ent->client->pers.passive != CRIP) // SH
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

// SH
				if (ent->client->pers.passive == INVIS)
				{
					ent->client->pers.passive_finished = level.time + 3;
				}
// \SH
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
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) || ent->ants > level.time)
		{
// SH
			if (ent->flags & FL_OBSERVER)
			{
				ent->client->latched_buttons &= ~BUTTON_ATTACK;
				MakePlayer(ent);
				return;
			}
// \SH
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
			ent->client->weaponstate = WEAPON_READY;
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
	int		vel;	// SH
	int		amount = ent->client->pers.weapon->quantity; // SH

// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%10 < 8)
		amount = 0;
// SH
	radius = damage+40;
	
//	if (is_quad)		// SH
//		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

// SH
	if (ent->client && (ent->client->pers.special == FSHOTS || ent->client->pers.special == TAFT))
		vel = 2000;
	else
		vel = 600;
// \SH	

	fire_grenade (ent, start, forward, damage, vel, 2.5, radius, 0);
// SH
	if (ent->client && ent->client->pers.special == AODEATH)
	{
		fire_grenade (ent, ent->angel->s.origin, forward, damage, vel, 2.5, radius, 0);
		amount *= (rand()%2)+1;
	}
// SH
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (ent->flags & FL_DEATHBLOW)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 5, 14, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
	else
		Weapon_Generic (ent, 5, 9, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
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
	int		vel; // SH
	int		amount = ent->client->pers.weapon->quantity; // SH
// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 9)
		amount = 0;

	if (ent->client && ent->client->pers.passive == ROBOT)
	{
		amount = 0;
		if (ent->wait == 0)
			damage = 60;
		else
			damage = 75;

		damage_radius = 0;
		radius_damage = 0;
	}
	else
// \SH
	{
		damage = 100 + (int)(random() * 20.0);
		radius_damage = 120;
		damage_radius = 120;
	}
	
//	if (is_quad)		// SH
//	{
//		damage *= 4;
//		radius_damage *= 4;
//	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
// SH
	if (ent->client && (ent->client->pers.special == FSHOTS || ent->client->pers.special == TAFT))
		vel = 2000;
	else if (ent->client && ent->client->pers.passive == ROBOT)
		vel = 850;
	else
		vel = 650;
// \SH	
	fire_rocket (ent, start, forward, damage, vel, damage_radius, radius_damage);
// SH
	if (ent->client && ent->client->pers.special == AODEATH)
	{
		fire_rocket (ent, ent->angel->s.origin, forward, damage, vel, damage_radius, radius_damage);
		amount *= (rand()%2)+1;
	}
// \SH
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (ent->flags & FL_DEATHBLOW)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}

void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};
	
	if (ent->client->pers.passive == ROBOT)
		Weapon_Generic (ent, 4, 8, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	else if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
	else
		Weapon_Generic (ent, 4, 8, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);


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
	int		vel;

//	if (is_quad)		// SH
//		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

// SH
	if (ent->client && ent->client->pers.passive == MAGE)
	{
		vec3_t	dir;
		VectorSet (dir, rand()%30-15, rand()%30-15, rand()%45-15);
		VectorNormalize2 (dir, dir);
		VectorScale (dir, 0.06, dir);
		MV (ent);
		VectorAdd (v_forward, dir, forward);
		damage = 28;
	}
// \SH

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

// SH
	if (ent->client && (ent->client->pers.special == FSHOTS || ent->client->pers.special == TAFT))
		vel = 2000;
	else
		vel = 1000;
// \SH	
	fire_blaster (ent, start, forward, damage, vel, effect, hyper);
	if (ent->client && ent->client->pers.special == AODEATH)
	{
		fire_blaster (ent, ent->angel->s.origin, forward, damage, vel, effect, hyper);
	}

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


void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	else
		Weapon_Generic (ent, 4, 6, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		amount = ent->client->pers.weapon->quantity; // SH

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 6)
		amount = 0;

	if (ent->client && ent->client->pers.passive == MAGE)
	{
		amount = 0;
		ent->client->weapon_sound = gi.soundindex("world/force1.wav");
	}

	if (ent->client && ent->client->pers.special == AODEATH)
	{
		amount *= (rand()%2)+1;
	}
// SH

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

// SH
			if (ent->client->ps.gunframe%2 == 0 || ent->client->pers.passive != MAGE)
// \SH
				Blaster_Fire (ent, offset, 20, true, effect);

// SH
			if (ent->flags & FL_DEATHBLOW)
			{
				ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
				if (ent->flags & FL_HIT)
				{
					ent->flags &= ~FL_HIT;
					ent->flags &= ~FL_DEATHBLOW;
					ent->client->pers.active_finished = level.time + 5;
					gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
				}
			}
// \SH
			else
				ent->client->pers.inventory[ent->client->ammo_index] -= amount;
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12 && ent->client->pers.passive != MAGE) // SH
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

	BoundAmmo(ent);
}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
	else
		Weapon_Generic (ent, 5, 11, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);

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
	int			db = 0; // SH
	int		amount = ent->client->pers.weapon->quantity; // SH
// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 10)
		amount = 0;
	else if (ent->client && ent->client->pers.passive == ROBOT)
	{
		amount = 0;
		damage = 12;
	}
//	else if (ent->client && ent->client->pers.special == SNIPER)
//	{
//		damage = 12;
//	}
// SH

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
//		damage *= 4;	// SH
		kick *= 4;
	}

	if (ent->flags & FL_DEATHBLOW) // SH
		db = 1;                    // SH

	if (ent->client && ent->client->pers.special != SNIPER && ent->client->pers.combo != C_ASS)	// SH
	{
		for (i=1 ; i<3 ; i++)
		{
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] = crandom() * 0.7;
		}
		ent->client->kick_origin[0] = crandom() * 0.35;
		ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	}
	
	// raise the gun as it is firing
	if (0) // SH !deathmatch->value && !(ent->client->pers.special == SNIPER))
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
// SH
	if (ent->client && ent->client->pers.special == AODEATH)
	{
		fire_bullet (ent, ent->angel->s.origin, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
		amount *= (rand()%2)+1;
	}
// \SH
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (db)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
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
	int			db = 0; // SH
	int		amount = ent->client->pers.weapon->quantity; // SH

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 6)
		amount = 0;

	if (ent->client && ent->client->pers.passive == CRIP)
	{
		damage = 9;
		amount = 0;
	}

	if (ent->client && ent->client->pers.special == AODEATH)
		amount *= (rand()%2)+1;

//	if (ent->client && ent->client->pers.special == SNIPER)
//		damage = 10;
//	else 
// SH

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

	if (ent->client->pers.inventory[ent->client->ammo_index] < ((shots * amount) + 1))
	{
		amount = 1;
		shots = (ent->client->pers.inventory[ent->client->ammo_index]) / amount;
	}

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
//		damage *= 4;		// SH
		kick *= 4;
	}

	if (ent->flags & FL_DEATHBLOW) // SH
		db = 1;                    // SH

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

//	for (i=0 ; i<shots ; i++)	// SH
//	{							// SH
		damage *= shots;
// \SH
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		start[2] += ent->viewheight-8;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
// SH
		if (ent->client && ent->client->pers.special == AODEATH)
		{
			fire_bullet (ent, ent->angel->s.origin, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
		}
// \SH
//	}	// SH
	
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (db)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * shots * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * shots;

	BoundAmmo(ent);
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

//	if (!(ent->client->pers.special == HASTE))  // SH
		Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
//	else
//		Weapon_Generic (ent, 4, 22, 61, 64, pause_frames, fire_frames, Chaingun_Fire);

}

// SH
void weapon_bopgun_fire (edict_t *ent)
{
	int amount = 1;
	int rail = 0, bfg = 0, ss = 0, gl = 0, rl = 0;
	int damage = 10;
	int rdamage = 0;
	int kick = 8;
	int db = 0;

	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

//gi.cprintf (ent, PRINT_HIGH, "========\n");

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
//		damage *= 4;	// SH
		kick *= 4;
	}

	if (ent->flags & FL_DEATHBLOW) 
	{
		db = 1;                    
		amount *= 10;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] >= amount)
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] -= amount;	
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("cells"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] >= amount)
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] -= amount;	
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("bullets"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] >= amount)
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] -= amount;	
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("bullets"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] >= amount)
	{
		rail = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] -= amount;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("slugs"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] >= amount)
	{
		ss = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] -= amount;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("shells"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg10k"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] >= amount)
	{
		bfg = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] -= amount;
		rdamage += 15;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("cells"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] >= amount)
	{
		gl = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] -= amount;
		rdamage += 15;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("grenades"));
		}
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] >= amount)
	{
		rl = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] -= amount;
		rdamage += 15;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] < ent->client->pers.inventory[ent->client->ammo_index] || ent->client->pers.inventory[ent->client->ammo_index] == 0)
		{
			ent->client->ammo_index = ITEM_INDEX(FindItem("rockets"));
		}
	}

	if (rail)
	{
		if (ss)
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[ROLL]  = ent->client->v_angle[ROLL];
			v[YAW]   = ent->client->v_angle[YAW];
			AngleVectors (v, forward, NULL, NULL);
			fire_fastbop (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_BOPGUN, TE_BLASTER);
			if (rl)
				fire_explobop (ent, start, forward, rdamage, MOD_BOPGUN);

			v[PITCH] = ent->client->v_angle[PITCH] + rand()%6 - 2.5;
			v[ROLL]  = ent->client->v_angle[ROLL] + rand()%6 - 2.5;
			v[YAW]   = ent->client->v_angle[YAW] + rand()%6 - 2.5;;
			AngleVectors (v, forward, NULL, NULL);
			fire_fastbop (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_BOPGUN, TE_BLASTER);
			if (rl)
				fire_explobop (ent, start, forward, rdamage, MOD_BOPGUN);

		}
		else
		{
			fire_fastbop (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_BOPGUN, TE_BLASTER);
			if (rl)
				fire_explobop (ent, start, forward, rdamage, MOD_BOPGUN);
		}
	}
	else
	{
		damage *= 5;
		damage = 100*rdamage + damage;

		if (ss)
		{
			v[PITCH] = ent->client->v_angle[PITCH];
			v[ROLL]  = ent->client->v_angle[ROLL];
			v[YAW]   = ent->client->v_angle[YAW];
			AngleVectors (v, forward, NULL, NULL);
			fire_blaster (ent, start, forward, damage, 1000, EF_BLASTER, 0);

			v[PITCH] = ent->client->v_angle[PITCH] + rand()%6 - 2.5;
			v[ROLL]  = ent->client->v_angle[ROLL] + rand()%6 - 2.5;
			v[YAW]   = ent->client->v_angle[YAW] + rand()%6 - 2.5;;
			AngleVectors (v, forward, NULL, NULL);
			fire_blaster (ent, start, forward, damage, 1000, EF_BLASTER, 0);
		}
		else
		{
			fire_blaster (ent, start, forward, damage, 1000, EF_BLASTER, 0);
		}
	}
// \SH
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (db)
	{
//gi.cprintf (ent, PRINT_HIGH, "used: %i\n", ent->client->pers.weapon->quantity * 10);	
//		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
//	else	
//		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}
// \SH

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
	int			db = 0; // SH
	int		amount = ent->client->pers.weapon->quantity; // SH

// SH

	if (ent->client->pers.special == BOPGUN)
	{
		weapon_bopgun_fire (ent);
		return;
	}

	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 9)
		amount = 0;
// SH

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
//		damage *= 4;	// SH
		kick *= 4;
	}

	if (ent->flags & FL_DEATHBLOW) // SH
		db = 1;                    // SH

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
// SH
	if (ent->client && ent->client->pers.special == AODEATH)
	{
		if (deathmatch->value)
			fire_shotgun (ent, ent->angel->s.origin, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
		else
			fire_shotgun (ent, ent->angel->s.origin, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

		amount *= (rand()%2)+1;
	}
// \SH
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (db)
	{
//gi.cprintf (ent, PRINT_HIGH, "used: %i\n", ent->client->pers.weapon->quantity * 10);	
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else	
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};
	int frames = 15;
	int amount = (ent->flags & FL_DEATHBLOW?10:1);

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] >= amount)
	{
		frames -= 2;
//		gi.cprintf (ent, PRINT_HIGH, "cells: %i ", ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]);	
//		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] -= amount;	
//		gi.cprintf (ent, PRINT_HIGH, "left: %i\n", ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]);	
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] >= amount)
	{
		frames -= 2;
//		gi.cprintf (ent, PRINT_HIGH, "bullets: %i ", ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]);	
//		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] -= amount;	
//		gi.cprintf (ent, PRINT_HIGH, "left: %i\n", ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]);	
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] && ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] >= amount)
	{
		frames -= 2;
//		gi.cprintf (ent, PRINT_HIGH, "bullets: %i ", ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]);	
//		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] -= amount;	
//		gi.cprintf (ent, PRINT_HIGH, "left: %i\n", ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]);	
	}
	BoundAmmo (ent);

	if ((ent->client->pers.special == HASTE || ent->client->pers.special == PUNISH))  // SH
		Weapon_Generic (ent, 7, 10, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
	else if (ent->client->pers.special == BOPGUN)
	{
		Weapon_Generic (ent, 7, frames, 36, 39, pause_frames, fire_frames, weapon_bopgun_fire);
	}
	else
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
	int			db = 0; // SH
	int		amount = ent->client->pers.weapon->quantity; // SH
// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 9)
		amount = 0;

// SH

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
//		damage *= 4;		// SH
		kick *= 4;
	}

	if (ent->flags & FL_DEATHBLOW) // SH
		db = 1;                    // SH

// SH
	if (ent->client->pers.special != SNIPER && ent->client->pers.combo != C_ASS)
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW] - 5;
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
// SH
		if (ent->client && ent->client->pers.special == AODEATH)
		{
			fire_shotgun (ent, ent->angel->s.origin, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		}
// \SH
		v[YAW]   = ent->client->v_angle[YAW] + 5;
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
// SH
		if (ent->client && ent->client->pers.special == AODEATH)
		{
			fire_shotgun (ent, ent->angel->s.origin, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
			amount *= (rand()%2)+1;
		}
// \SH
	}
	else
	{
		v[PITCH] = ent->client->v_angle[PITCH];
		v[YAW]   = ent->client->v_angle[YAW];
		v[ROLL]  = ent->client->v_angle[ROLL];
		AngleVectors (v, forward, NULL, NULL);
		fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
		fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (db)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
	else
		Weapon_Generic (ent, 6, 11, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
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
	int			db = 0; // SH
	int		amount = ent->client->pers.weapon->quantity; // SH
// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 9)
		amount = 0;
	if (ent->client && ent->client->pers.passive == MAGE)
		amount = 0;
// SH

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
// SH
	if (ent->client && ent->client->pers.passive == MAGE)
	{
		damage = 25;
		kick = 50;
	}
// \SH
	if (is_quad)
	{
//		damage *= 4;	// SH
		kick *= 4;
	}

	if (ent->flags & FL_DEATHBLOW) // SH
		db = 1;                    // SH

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);
// SH
	if (ent->client && (ent->client->pers.special == AODEATH || ent->client->pers.passive == MAGE))
	{
		fire_rail (ent, ent->angel->s.origin, forward, damage, kick);
		amount *= (rand()%2)+1;
	}
// \SH
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (db)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
	else
		Weapon_Generic (ent, 3, 10, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
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
	int		damage = 250;
	float	damage_radius = 700;
	int		vel;
	int		amount = ent->client->pers.weapon->quantity; // SH
// SH
	if (ent->client && (ent->client->pers.special == AMMO || ent->client->pers.special == PUNISH) && rand()%12 < 7)
		amount = 0;
// SH

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

//	if (is_quad)		// SH
//		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
// SH
	if (ent->client && (ent->client->pers.special == FSHOTS || ent->client->pers.special == TAFT))
		vel = 2000;
	else
		vel = 400;
// \SH	
	fire_bfg (ent, start, forward, damage, vel, damage_radius);
// SH
	if (ent->client && ent->client->pers.special == AODEATH)
	{
		fire_bfg (ent, ent->angel->s.origin, forward, damage, vel, damage_radius);
		amount *= (rand()%2)+1;
	}
// \SH
	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

// SH
	if (ent->flags & FL_DEATHBLOW)
	{
		ent->client->pers.inventory[ent->client->ammo_index] -= amount * 10;
		if (ent->flags & FL_HIT)
		{
			ent->flags &= ~FL_HIT;
			ent->flags &= ~FL_DEATHBLOW;
			ent->client->pers.active_finished = level.time + 5;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
		}
	}
// \SH
	else
		ent->client->pers.inventory[ent->client->ammo_index] -= amount;

	BoundAmmo(ent);
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	if ((ent->client->pers.special != HASTE && ent->client->pers.special != PUNISH))  // SH
		Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
	else
		Weapon_Generic (ent, 8, 20, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);

}


//======================================================================
