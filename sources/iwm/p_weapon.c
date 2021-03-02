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

qboolean	is_quad;
byte		is_silenced;

void fire_screwdriver (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_gravgrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_infestgrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_flare (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_forcegrenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_jackexpl (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rotatelas (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int timer, float damage_radius, qboolean held);


void weapon_grenade_fire (edict_t *ent, qboolean held);

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result, int left)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (left == 1)
	{
		_distance[1] *= -1;
	}
	else if (left == 2)
	{
		_distance[1] = 0;
	}
	else
	{
		if (client)
		{
			if (client->pers.hand == LEFT_HANDED)
				_distance[1] *= -1;
			else if (client->pers.hand == CENTER_HANDED)
				_distance[1] = 0;
		}
	}
	G_ProjectSource (point, _distance, forward, right, result);
}

void P_ProjectSource_Dev (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result, int left)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (left == 1)
	{
		_distance[1] *= -1;
	}
	/*if (left == 1)
	{
		_distance[1] *= -1;
	}
	else if (left == 2)
	{
		_distance[1] = 0;
	}
	else
	{
		if (client)
		{
			if (client->pers.hand == LEFT_HANDED)
				_distance[1] *= -1;
			else if (client->pers.hand == CENTER_HANDED)
				_distance[1] = 0;
		}
	}*/
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

	//if (deathmatch->value)
	//	return;

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

qboolean CheckWeaponBans (gitem_t *i)
{
	if (Q_stricmp(i->owned_item, "Shotgun") == 0)
	{
		if (shotgun_ban->string && Q_stricmp(shotgun_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(shotgun_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Super Shotgun") == 0)
	{
		if (ssg_ban->string && Q_stricmp(ssg_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(ssg_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Machinegun") == 0)
	{
		if (machinegun_ban->string && Q_stricmp(machinegun_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(machinegun_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Chaingun") == 0)
	{
		if (chaingun_ban->string && Q_stricmp(chaingun_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(chaingun_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Grenade Launcher") == 0)
	{
		if (gl_ban->string && Q_stricmp(gl_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(gl_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Grenades") == 0)
	{
		if (grenade_ban->string && Q_stricmp(grenade_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(grenade_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Rocket Launcher") == 0)
	{
		if (rl_ban->string && Q_stricmp(rl_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(rl_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Hyperblaster") == 0)
	{
		if (hyperblaster_ban->string && Q_stricmp(hyperblaster_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(hyperblaster_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "Railgun") == 0)
	{
		if (railgun_ban->string && Q_stricmp(railgun_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(railgun_ban, i->bannumber))
			return true;
	}

	if (Q_stricmp(i->owned_item, "BFG10k") == 0)
	{
		if (bfg_ban->string && Q_stricmp(bfg_ban->string, "all") == 0)
			return true;

		if (i->bannumber && CheckBan(bfg_ban, i->bannumber))
			return true;
	}

	return false;
}

void Pickup_All_Grenades (edict_t *ent)
{
	int i = 0;
	gitem_t *cur;
		
	for (; i < game.num_items; i++)
	{
		cur = &itemlist[i];

		if (!cur)
			continue;
		if (cur->pickup_name)
		{
			if (cur->owned_item && Q_stricmp(cur->owned_item, "Grenades") == 0)
			{
				if (!CheckBan(grenade_ban, cur->bannumber))
				ent->client->pers.inventory[ITEM_INDEX(cur)] = 1;
			}
		}
	}
}

qboolean PickupLinkedWeapons(edict_t *item, edict_t *picker)
{
	int n;
	gitem_t *it = itemlist;
	gitem_t *i;
	gclient_t *client = picker->client;
	int old;

	for (n = 0; n < game.num_items; n++, it++)
	{
		if (!itemlist[n].weaponthink)
			continue;
		if (!itemlist[n].owned_item)
			continue;
		if (Q_stricmp(itemlist[n].pickup_name, itemlist[n].owned_item) == 0)
			continue;
		if (CheckWeaponBans(&itemlist[n]))
			continue;
		if (Q_stricmp(itemlist[n].owned_item, item->item->pickup_name) == 0)
		{
			picker->client->pers.inventory[ITEM_INDEX(it)]++;
		}
	}
	// GL hack
	if (Q_stricmp(item->item->pickup_name, "Grenade Launcher") == 0 || Q_stricmp(item->item->pickup_name, "Grenades") == 0)
	{
		Pickup_All_Grenades(picker);
	}
	return true;
}

qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;
	edict_t *oldent;
	edict_t *newentity = NULL;
	int is_newitem = 0;

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
		if (ent->item->ammo)
		{
			ammo = FindItem (ent->item->ammo);
			if ( (int)dmflags->value & DF_INFINITE_AMMO )
				Add_Ammo (other, ammo, 1000);
			else
				Add_Ammo (other, ammo, ammo->quantity);
		}

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
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") || other->client->pers.weapon == FindItem("screwdriver launcher") || other->client->pers.weapon == FindItem("saw-toothed hammer") || other->client->pers.weapon == FindItem("battery-operated drill")  ) )
		other->client->newweapon = ent->item;

	if (ent->item->owned_item)
	{
		newentity = G_Spawn();
		newentity->inuse = true;
		oldent = ent;
		other->client->pers.inventory[index]--;
		newentity->item = FindItem (oldent->item->owned_item);
		gi.linkentity(newentity);
		ent = newentity;
		other->client->pers.inventory[ITEM_INDEX(FindItem(oldent->item->owned_item))]++;
		is_newitem = 1;
	}

	if (!PickupLinkedWeapons(ent, other))
	{
		gi.dprintf ("PickupLinkedWeapons: Weapon has no links, or returned false somehow!\n");
	}

	if (is_newitem)
		G_FreeEdict (newentity);
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

	if (ent->client->dkhammer_framenum > level.framenum)
		return;

	if (ent->vehicle)
		return;

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
	if (ent->s.modelindex == 255 || (ent->s.modelindex == 255 && ent->client->cloak_turninvis_framenum)) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

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
	ent->client->ps.gunindex = ModelIndex(ent->client->pers.weapon->view_model);

	ent->client->agm_charge = 0;
	ent->client->agm_showcharge = false;
	ent->client->agm_tripped = false;
	ent->client->agm_on = false;
	ent->client->agm_push = false;
	ent->client->agm_pull = false;
	ent->client->agm_target = NULL;

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
	if ((iwm_gamemode->value == 1 && ent->client->resp.team == -1))
		return;
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

		// Grenade HASTE!
		if (CTFApplyHaste(ent) && (Q_stricmp(ent->client->pers.weapon->view_model, "models/weapons/v_handgr/tris.md2") == 0 || ent->client->pers.weapon == FindItem("Trap") || ent->client->pers.weapon == FindItem("Tesla")))
			ent->client->pers.weapon->weaponthink (ent);
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
	char *name = NULL;

	if (ent->vehicle)
		return;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			safe_cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			safe_cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;

	if (Q_stricmp(item->pickup_name, "Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 1\n");
		ent->client->pers.grenadetype = 1;
	}
	else if (Q_stricmp(item->pickup_name, "Gravity Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 2\n");
		ent->client->pers.grenadetype = 2;
	}
	else if (Q_stricmp(item->pickup_name, "Concussion Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 3\n");
		ent->client->pers.grenadetype = 3;
	}
	else if (Q_stricmp(item->pickup_name, "Infest Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 4\n");
		ent->client->pers.grenadetype = 4;
	}
	else if (Q_stricmp(item->pickup_name, "Rotating Laser Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 5\n");
		ent->client->pers.grenadetype = 5;
	}
	else if (Q_stricmp(item->pickup_name, "Area Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 6\n");
		ent->client->pers.grenadetype = 6;
	}
	else if (Q_stricmp(item->pickup_name, "Exploding Jack Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 7\n");
		ent->client->pers.grenadetype = 7;
	}
	else if (Q_stricmp(item->pickup_name, "Flash Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 8\n");
		ent->client->pers.grenadetype = 8;
	}
	else if (Q_stricmp(item->pickup_name, "Glowsticks") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 9\n");
		ent->client->pers.grenadetype = 9;
	}
	else if (Q_stricmp(item->pickup_name, "Gib Mines") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 10\n");
		ent->client->pers.grenadetype = 10;
	}
	else if (Q_stricmp(item->pickup_name, "Spikebombs") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 10\n");
		ent->client->pers.grenadetype = 11;
	}
	else if (Q_stricmp(item->pickup_name, "Earthquake Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 10\n");
		ent->client->pers.grenadetype = 12;
	}
	else if (Q_stricmp(item->pickup_name, "Teleport Grenades") == 0)
	{
		//safe_cprintf (ent, PRINT_HIGH, "Grenade type 10\n");
		ent->client->pers.grenadetype = 13;
	}

	if (Q_stricmp(item->pickup_name, "Grenade Launcher") == 0)
	{
		int gt = ent->client->pers.grenadetype;
		
		if (gt == 1)
			name = "Grenade Launcher";
		else if (gt == 2)
			name = "Gravity Grenade Launcher";
		else if (gt == 3)
			name = "Concussion Grenade Launcher";
		else if (gt == 4)
			name = "Infest Grenade Launcher";
		else if (gt == 5)
			name = "Rotating Laser Grenade Launcher";
		else if (gt == 6)
			name = "Area Grenade Launcher";
		else if (gt == 7)
			name = "Exploding Jack Grenade Launcher";
		else if (gt == 8)
			name = "Flash Grenade Launcher";
		else if (gt == 9)
			name = "Glowstick Grenade Launcher";
		else if (gt == 10)
			name = "Gib Mine Launcher";
		else if (gt == 11)
			name = "Spikebomb Launcher";
		else if (gt == 12)
			name = "Earthquake Grenade Launcher";
		else if (gt == 13)
			name = "Teleport Grenade Launcher";
	}
	else
		name = item->pickup_name;

	if (!ent->client->weap_s)
		safe_cprintf (ent, PRINT_HIGH, "%s\n", name);
	else
		ent->client->weap_s = 0;
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
		safe_cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}


void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	gclient_t *client;
	int index;

	client = ent->client;
	if (!client)
		return;

//	if (ent->client->silencer_shots)
//		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("regeneration");
	if (tech && ent->client->reg_framenum > level.framenum) 
	{
		if (client->ctf_regentime < level.time) 
		{
			client->ctf_regentime = level.time + 1;
			
			if (ent->health < 200) 
			{
				ent->health += 5;
				if (ent->health > 200)
					ent->health = 200;
			}
			index = ArmorIndex (ent);
			if (index && client->pers.inventory[index] < 200) 
			{
				client->pers.inventory[index] += 5;
				if (client->pers.inventory[index] > 200)
					client->pers.inventory[index] = 200;
			}
		}
	}
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
qboolean pig_gettarg (edict_t *ent);
void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;



	if(ent->deadflag || (ent->s.modelindex != 255 && !ent->client->cloak_framenum && !ent->vehicle)) // VWep animations screw up corpses
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
		//if (ent->client->ps.gunframe == 0)
		//	gi.sound (ent, CHAN_BODY, SoundIndex("weapons/hyprbu1a.WAV"), 0.4, ATTN_NORM, 0);
		
		if (Q_stricmp(ent->client->pers.weapon->view_model, "models/weapons/v_shotg/tris.md2") == 0 && ent->client->ps.gunframe == 0)
			gi.sound (ent, CHAN_BODY, SoundIndex("weapons/Shotgr1b.wav"), 0.4, ATTN_NORM, 0);

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
			if (!ent->client->pers.view_rocket) // new line!!
			{ // if (pers.view_rocket)   <---new line!!
				if ((!ent->client->ammo_index) || (ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
				{
					if ((ent->client->pers.weapon == FindItem("PIG2K")) && !pig_gettarg(ent))
						return;

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
						gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
						ent->pain_debounce_time = level.time + 1;
					}
					NoAmmoWeaponChange (ent);
				}
			} // if (pers.view_rocket)  <---new line!!
		}
		else
		{
			//ent->client->pers.view_rocket = false; //new line
			//ent->chasetarget = NULL; //new line


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
		if (!ent->client->pers.view_rocket) // new line
		{ // new line
			for (n = 0; fire_frames[n]; n++)
			{
				if (ent->client->ps.gunframe == fire_frames[n])
				{
					
					ent->client->cloak_turninvis_framenum = level.framenum + 20;
					//ZOID
					if (!CTFApplyStrengthSound(ent))
						//ZOID
						if (ent->client->quad_framenum > level.framenum)
							gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage3.wav"), 1, ATTN_NORM, 0);
						//ZOID
						CTFApplyHasteSound(ent);
						//ZOID
						
						if (ent->client->pers.weapon == FindItem ("rocket launcher") && ent->client->pers.guided) //new line
							ent->client->pers.view_rocket = true; //new line
						fire (ent);
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
						
						if (ent->client->quad_framenum > level.framenum)
							gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage3.wav"), 1, ATTN_NORM, 0);
						
						break;
				}
			}
		}
		else 
		{
			// new code starts here
			if (!(ent->client->buttons & BUTTON_ATTACK) && !(ent->client->oldbuttons & BUTTON_ATTACK))
			{
				//ent->client->pers.view_rocket = false;
				//ent->chasetarget = NULL;
			}
			// new code ends here
			return;
		}


		if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Devastator") == 0)
		{
			if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
			{
				if (ent->client->ps.gunframe == 8)
					ent->client->ps.gunframe = 2;

				// Hax
				if (ent->client->ps.gunframe == 6)
					ent->client->ps.gunframe++;
			}

			if (!((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
			{
				if (ent->client->ps.gunframe == 8)
					ent->client->ps.gunframe = 10;

				if (ent->client->ps.gunframe == 5)
					ent->client->ps.gunframe = 10;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//ZOID
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	if (ent->frozen)
		return;

	if (ent->client->star_framenum > level.framenum)
		return;

	Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
		fire_frames, fire);

	// run the weapon frame again if hasted
	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;

	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate != WEAPON_FIRING))
		&& oldstate == ent->client->weaponstate) {
		Weapon_Generic2 (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames, 
			fire_frames, fire);
	}
}
//ZOID

/*
======================================================================

GRENADE

======================================================================
*/


void fire_flash (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_tesla (edict_t *self, vec3_t start, vec3_t aimdir, int damage_multiplier, int speed);

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

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	if (ent->client->pers.weapon == FindItem("Tesla"))
	{
//		VectorSet(offset, 0, -12, ent->viewheight-26);
		VectorSet(offset, 0, -4, ent->viewheight-22);
	}
	else
	{
//		VectorSet(offset, 8, 8, ent->viewheight-8);
		VectorSet(offset, 2, 6, ent->viewheight-14);
	}
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	if (ent->client->pers.weapon == FindItem("Tesla"))
		fire_tesla (ent, start, forward, 1, speed);
	else
		fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || (ent->s.modelindex != 255 && !ent->client->cloak_turninvis_framenum)) // VWep animations screw up corpses
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
					gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
			gi.sound(ent, CHAN_WEAPON, SoundIndex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = SoundIndex("weapons/hgrenc1b.wav");
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
void fire_concussiongrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_spikebomb (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_ngrenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer);
void fire_gasgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_gibmine (edict_t *self, vec3_t start, vec3_t aimdir);
void fire_earthquake_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_prox (edict_t *self, vec3_t start, vec3_t aimdir, int damage_multiplier, int speed);
void fire_chaingren (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer);
void fire_telegrenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;
	int gt = ent->client->pers.grenadetype;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Prox Launcher") == 0)
	{
		damage = 90;
		fire_prox (ent, start, forward, 1, 600);
	}
	else
	{
		if (gt == 1)
			fire_grenade (ent, start, forward, damage, 600, 2.5, radius);
		else if (gt == 2)
			fire_gravgrenade (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 3)
			fire_concussiongrenade (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 4)
			fire_infestgrenade2 (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 5)
			fire_rotatelas (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 6)
			fire_forcegrenade2 (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 7)
			fire_jackexpl (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 8)
			fire_flash (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 9)
			fire_flare (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 10)
			fire_gibmine (ent, start, forward);
		else if (gt == 11)
			fire_spikebomb (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 12)
			fire_earthquake_grenade (ent, start, forward, damage, 600, 2.5, radius, false);
		else if (gt == 13)
			fire_telegrenade (ent, start, forward, damage, 600, 2.5, radius, false);
	}

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

void Weapon_ProxLauncher (edict_t *ent)
{
	static int      pause_frames[]  = {34, 51, 59, 0};
	static int      fire_frames[]   = {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/
void fire_freeze (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_womd (edict_t *self, vec3_t start, vec3_t dir, int speed);

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	if (ent->client->pers.weapon == FindItem ("Weapon of Mass Destruction") && ent->client->pers.womded && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		safe_cprintf (ent, PRINT_HIGH, "You already WOMDed this life!\n");
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 50 && ent->client->pers.weapon == FindItem ("Weapon of Mass Destruction"))
	{
		ent->client->ps.gunframe = 13;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	if (ent->client->pers.weapon == FindItem ("Freeze Nuke Launcher"))
		fire_freeze (ent, start, forward, damage, 650, damage_radius, radius_damage);
	else if (ent->client->pers.weapon == FindItem ("Weapon of Mass Destruction"))
		fire_womd (ent, start, forward, 385);
	else
		fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		if (ent->client->pers.weapon == FindItem ("Weapon of Mass Destruction"))
			ent->client->pers.inventory[ent->client->ammo_index] -= 50;
		else
			ent->client->pers.inventory[ent->client->ammo_index] --;
	}
	else
	{
		if (ent->client->pers.weapon == FindItem ("Weapon of Mass Destruction"))
			ent->client->pers.womded = 1;
	}
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
void fire_lasertraplaser (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hy);

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hy, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->pers.weapon != FindItem("Neutron Blaster"))
		fire_blaster (ent, start, forward, damage, 1000, effect, hy);
	else
	{
		float old = ent->tempfloat;
		ent->tempfloat = 666.767676;
		fire_blaster2 (ent, start, forward, damage, 1000, effect, true);
		ent->tempfloat = old;
	}
	//fire_pacman (ent, start, forward, 0, 400, 0, false);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	if (hy)
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

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = SoundIndex("weapons/hyprbl1a.wav");

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
				gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
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
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
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
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

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
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = SoundIndex("weapons/chngnl1a.wav");
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
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

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
	int			kick = 60;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

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
void weapon_trailgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int damage, kick;

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

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 3;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_rail (ent, start, forward, damage, kick, 0);
	v[YAW]   = ent->client->v_angle[YAW] + 3;
	AngleVectors (v, forward, NULL, NULL);
	fire_rail (ent, start, forward, damage, kick, 0);
	v[YAW]   = ent->client->v_angle[YAW];
	v[PITCH] = ent->client->v_angle[PITCH] - 3;
	AngleVectors (v, forward, NULL, NULL);
	fire_rail (ent, start, forward, damage, kick, 0);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 3;
}

/*******mod_dist_point_to_line***********

* Simply calculates the distance from a point to a line

* point: the point for which we wish to calculate the distance

* linepoint: a point on the line

* linedir: the direction of the line

*****************************************/

float mod_dist_point_to_line(vec3_t point, vec3_t linepoint, vec3_t linedir)

{

        vec3_t temp, temp2;

 

        VectorSubtract(point, linepoint, temp);

        CrossProduct(temp, linedir, temp2);

 

        return VectorLength(temp2)/VectorLength(linedir);

}

 

/*******mod_GetLeadoffVec****************

* Author: Tim Matthews

* NOTE: This Code may be distributed and used freely as long as you give due credit to the author.

*

* Get's the direction in which to shoot at an enemy, taking into account his

* current velocity

*

*Arguments

*       self: you

*       fireorigin: the point from which the blast originates

*       rad: the radius of the bounding sphere. Only consider monsters inside this sphere

*       projspeed: the speed of the projectile being fired

*       bydist: true if judging best target by distance, otherwise test by angle

*       firedir: the direction we want to calculate

*************************************/

edict_t *mod_GetLeadoffVec(edict_t *self, vec3_t fireorigin, float rad, float projspeed, int bydist, vec3_t firedir)

{

        edict_t *other; /*any potential target besides yourself*/

        edict_t *best /*best victim so far*/;

 

        vec3_t viewvec;  /*your line of site*/

        vec3_t guessdir;

        vec3_t bestvec; /*the best guess for the direction of your blaster fire*/

        float bestdist = 0, dist;  /*distance of the other guy and the shortest distance encountered*/

 

 

        vec3_t temp, temp2, otherdir;

        float d, t;

        double alpha, beta, rho;

        double a, b, c, t1, t2;

 

        //gi.dprintf("Checking Targets\n");

 

        AngleVectors (self->client->v_angle, viewvec, NULL, NULL); /*get the view direction*/

 

        best=NULL;

        other=findradius(NULL, fireorigin, rad); /*find something*/

 

        while (other) {

                 if (other->health>0 && other!=self) { /*might have to modify these*/

                         if (visible(self, other) && infront(self, other)) {

                                  /*player is in front and visible*/

 

                                  /*calculate lead off*/

                                  VectorSubtract(other->s.origin, other->s.old_origin, otherdir);

                                  alpha=VectorNormalize(otherdir); /*alpha = speed,  otherdir=direction vector*/

 

                                  if (alpha>.05 && projspeed>0) { /*if speed is significant, this value may have to be changed*/

                                          d=mod_dist_point_to_line(fireorigin, other->s.origin, otherdir); /*distance from the firepoint to the

                                                                                                                                                                                                                                                                                        the line on which the enemy is running*/

        

                                          beta=projspeed; /*our projectile speed*/

 

                                          VectorSubtract(fireorigin, other->s.origin, temp);

                                          CrossProduct(temp, otherdir, temp2);  //temp2 now holds the normal to a plane defined by fireorigin and other->s.origin

                                          CrossProduct(temp2, otherdir, temp);

                                          VectorNormalize(temp); /*temp holds the direction from the point to the line*/

 

                                          VectorScale(temp, d, guessdir);

                                          VectorAdd(guessdir, fireorigin, guessdir);

                                          VectorSubtract(guessdir, other->s.origin, guessdir);

                                          rho=VectorLength(guessdir); /*the length from other->s.origin to the point where the perpendicular vector from fireorigin intersects*/

 

                                          /*now, a little quadratic equation solving...*/

                                          a=alpha*alpha-beta*beta; 

                                          b=-2*alpha*rho;

                                          c=rho*rho+d*d;

 

                                          t1=(-b+sqrt(b*b-4*a*c))/(2*a);

                                          t2=(-b-sqrt(b*b-4*a*c))/(2*a);

 

                                          t=(t1>=0)?t1:t2; /*positive solution is the correct one*/

                                  

                                          VectorScale(otherdir, t*alpha, guessdir);

                                          VectorAdd(other->s.origin, guessdir, guessdir);

                                          VectorSubtract(guessdir, fireorigin, guessdir); /*now we have our best guess*/

                                  } else {

                                          /*enemy is standing still, so just get a simple direction vector*/

                                    VectorSubtract(other->s.origin, fireorigin, guessdir); 

                                  }

 

 

                                  if (bydist) {

                                          dist=guessdir[0]*guessdir[0]+guessdir[1]*guessdir[1]+guessdir[2]*guessdir[2];

 

                                          if (!best || dist<bestdist) {

                                                   best=other;

                                                   VectorCopy(guessdir, bestvec);

                                                   bestdist=dist;

                                          }

                                  } else {

                                          /*choose best as the person most in front of us*/

                                          VectorNormalize(guessdir);

                                          dist=DotProduct(viewvec, guessdir);

 

                                          if (!best || dist>bestdist) {

                                                   best=other;

                                                   VectorCopy(guessdir, bestvec);

                                                   bestdist=dist;

                                          }

                                 }

                         }

                 }

                 other=findradius(other, self->s.origin, rad); /*find the next entity*/

        }

        

        if (!best) /*No targets aquired, so just fire forward as usual*/

                 AngleVectors(self->client->v_angle, firedir, NULL, NULL);

        else {

                 //gi.dprintf("Target %s Aquired\n", best->classname);

                 VectorCopy(bestvec, firedir);

                 VectorNormalize(firedir);

        }

 

        return best;

}

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
//	vec3_t		v;

	if (ent->client->pers.weapon == FindItem("Triple Slug Railgun"))
	{
		weapon_trailgun_fire(ent);
		return;
	}

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

	if (!ent->vehicle)
	{

		AngleVectors (ent->client->v_angle, forward, right, NULL);

		VectorScale (forward, -3, ent->client->kick_origin);
		ent->client->kick_angles[0] = -3;

		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
			
		fire_rail (ent, start, forward, damage, kick, 0);
	
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_RAILGUN | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	else
	{
		edict_t *self = ent->vehicle;
//		vec3_t	start2;
//		vec3_t	dir2;
//		vec3_t	forward2, right2;

		AngleVectors (self->s.angles, forward, right, NULL);

		VectorScale (forward, -3, ent->client->kick_origin);
		ent->client->kick_angles[0] = -3;

		VectorSet(offset, 0, 7,  self->viewheight-20);
		P_ProjectSource (NULL, self->s.origin, offset, forward, right, start, 0);

		if (self->weapnum == 1)
		{
			//monster_fire_railgun (self, start, forward, 50, 100, MZ2_GLADIATOR_RAILGUN_1);
			fire_rail (self, start, forward, 50, 100, -1);
		
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_RAILGUN | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		else if (self->weapnum == 2)
		{
			fire_rocket (self, start, forward, 45, 500, 100, 100);
		
			// send muzzle flash
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_ROCKET | is_silenced);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
	}

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

/*
-----------------
IWM
===
Weapon routines
-----------------
*/

float _rndnum (float y, float z)
{
	return ((random()*((z)-((y)+1)))+(y));
}


// Chain Grenade Launcher

void ChainGren_Fire (edict_t *ent)
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
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

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
		gi.sound(ent, CHAN_AUTO, SoundIndex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = SoundIndex("weapons/chngnl1a.wav");
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
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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

	//for (i=0 ; i<shots ; i++)
	//{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

		fire_grenade (ent, start, forward, 60, 600, 2.5, 60);
	//}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 1;
}


void Weapon_ChainGren (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 7, 9, 11, 13, 15, 17, 19, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, ChainGren_Fire);
}


void Weapon_CO2_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 2;
	int			kick = 650;
	vec3_t	kvel;
	float	mass;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (ent, start, forward, damage, kick, 40, 40, MOD_BLASTER);


	if (ent->mass < 50)
		mass = 50;
	else
		mass = ent->mass;

	VectorInverse(forward);
	
	if (!is_quad)
		VectorScale (forward, 250.0 * 2, kvel);
	else
		VectorScale (forward, 250.0 * 8, kvel);

	VectorAdd (ent->velocity, kvel, ent->velocity);

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

void Weapon_CO2 (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 11, 36, 39, pause_frames, fire_frames, Weapon_CO2_Fire);
}


void Weapon_Mace_Fire (edict_t *ent)
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
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_melee (ent, start, forward, 80, 500, 100, 1, MOD_MACE); // FIXME Mod

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_SHOTGUN | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

}

void Weapon_Mace (edict_t *ent)
{
	static int	pause_frames[]	= {16, 17, 0};
	static int	fire_frames[]	= {11, 0};

	Weapon_Generic (ent, 4, 15, 17, 20, pause_frames, fire_frames, Weapon_Mace_Fire);
}

void Weapon_Drill_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_melee (ent, start, forward, 45, 8, 5, 1, MOD_DRILL); // FIXME Mod

	if (!ent->client->weapon_sound)
		ent->client->weapon_sound = SoundIndex("weapons/drill_loop.wav");

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_SHOTGUN | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

}

void Weapon_Drill (edict_t *ent)
{
	static int	pause_frames[]	= {20, 22, 0};
	static int	fire_frames[]	= {10, 11, 12, 13, 14, 0};

	Weapon_Generic (ent, 2, 19, 22, 25, pause_frames, fire_frames, Weapon_Drill_Fire);

	if (ent->client->ps.gunframe == 4)
	{
		//gi.dprintf ("Up?\n");
		gi.sound (ent, CHAN_AUTO, SoundIndex("weapons/drill_warmup.wav"), 1, ATTN_NORM, 0);
	}

	if (ent->client->ps.gunframe == 15)
	{
		ent->client->weapon_sound = 0;
		gi.sound (ent, CHAN_AUTO, SoundIndex("weapons/drill_warmdown.wav"), 1, ATTN_NORM, 0);
	}

	if (ent->client->ps.gunframe == 14 && ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
		ent->client->ps.gunframe = 10;
}

void G_Spawn_Splash(int type, int count, int color, vec3_t start, vec3_t movdir, vec3_t origin ) ;

#define	MASK_LIGHTNING (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)

double round(double x)
{
	if(x >= 0.5)
	{
		return ceil(x);
	}
	else
	{
		return floor(x);
	}
}

#define COLOR_RED	  0xf2f2f0f0//red
#define COLOR_GREEN	  0xd0d1d2d3//green
#define COLOR_BLUE	  0xf3f3f1f1//blue
#define COLOR_YELLOW	  0xdcdddedf//yellow
#define COLOR_YELLOW2	  0xe0e1e2e3//yellow strobe
#define COLOR_PURPLE	  0x80818283//dark purple
#define COLOR_BLUE2	  0x70717273//light blue
#define COLOR_GREEN2	  0x90919293//different green
#define COLOR_PURPLE2	  0xb0b1b2b3//purple
#define COLOR_RED2	  0x40414243//different red
#define COLOR_ORANGE	  0xe2e5e3e6//orange
#define COLOR_MIX	  0xd0f1d3f3//mixture
#define COLOR_INNER1	  0xf2f3f0f1//inner = red, outer = blue
#define COLOR_INNER2	  0xf3f2f1f0//inner = blue, outer = red
#define COLOR_INNER3	  0xdad0dcd2//inner = green, outer = yellow
#define COLOR_INNER4	  0xd0dad2dc //inner = yellow, outer = green

void Weapon_Thunderbolt_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
//	vec3_t		v;

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
	
	//VectorScale (forward, -3, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -3;
	
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	if (ent->client->ps.gunframe == 12)
	{
		vec3_t  start; 
		vec3_t  forward; 
		vec3_t  end; 
		trace_t tr; 
		vec3_t endp;
		vec3_t g;
		vec3_t down = {0, 0, 20};
		float dist;
		vec3_t dis;
		
		VectorCopy(ent->s.origin, start); 
		// Copy your location 
	//	start[2] += ent->viewheight; 
		// vector for start is at your height of view 
	//	AngleVectors(ent->client->v_angle, forward, NULL, NULL); 
		AngleVectors (ent->client->v_angle, forward, right, NULL);
	
		VectorScale (forward, -3, ent->client->kick_origin);
		ent->client->kick_angles[0] = -3;
	
		VectorSet(offset, 0, 7,  ent->viewheight+3);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
		// Angles 
		VectorMA(start, 8192, forward, end); 
		// How far will the line go? 
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_LIGHTNING); 
		// Trace the line



		VectorCopy (tr.endpos, g);
		VectorSubtract (g, down, g);

		gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/light_hit.wav"), 0.6, ATTN_NORM, 0);

		//gi.dprintf ("%i\n", gi.pointcontents(g));
		//gi.dprintf ("%f %f %f %f %f\n", round(0.8192), round(0.0454), round(0.0222), round(0.0054), round(0.0001));

		if (gi.pointcontents(g) & 32)
		{
			edict_t *bl2 = NULL;

			while (bl2 = findradius(bl2, ent->s.origin, 8126))
			{
				if (!bl2->inuse)
					continue;
				if (!bl2->takedamage)
					continue;
				if (bl2->solid != SOLID_BBOX)
					continue;
				if (!bl2->waterlevel > 0)
					continue;

				T_Damage (bl2, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 999, 999, 0, MOD_LIGHTNING_BOLTDISCHARGE); // FIXME Mod
			}
		}

		if (!ent->waterlevel)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_LIGHTNING);
			gi.WriteShort (tr.ent - g_edicts);			// destination entity
			gi.WriteShort (ent - g_edicts);		// source entity
			gi.WritePosition (tr.endpos);
			gi.WritePosition (start);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			VectorCopy (tr.endpos, endp);

			VectorMA (endp, -0.02, tr.plane.normal, endp);
			G_Spawn_Splash (TE_TUNNEL_SPARKS, 50, COLOR_YELLOW2, endp, endp, endp);
			G_Spawn_Splash (TE_TUNNEL_SPARKS, 50, COLOR_RED2, endp, endp, endp);

			VectorSubtract (start, tr.endpos, dis);
			VectorInverse (dis);
			dist = VectorLength (dis);
			dist /= 10000;
			dist *= 3;

			//gi.dprintf ("%f\n", dist);

			if (tr.ent && tr.ent->takedamage)
			{
				T_Damage (tr.ent, ent, ent, vec3_origin, tr.ent->s.origin, vec3_origin, 80, 0, DAMAGE_ENERGY|DAMAGE_NO_ARMOR, MOD_THUNDERBOLT); // FIXME Mod
				gi.sound (tr.ent, CHAN_AUTO, SoundIndex ("weapons/light_hit.wav"), 1, ATTN_NORM, 0);
				if (tr.ent->fire_framenum && tr.ent->fire_entity)
					tr.ent->fire_framenum += level.framenum + 40;
				else
				{
					CreateStickingFire(ent, tr.ent->s.origin, tr.ent);
					tr.ent->fire_framenum = level.framenum + 100;
				}
			}
			else
				gi.positioned_sound (tr.endpos, g_edicts, CHAN_AUTO, SoundIndex ("weapons/light_hit.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			edict_t *bl = NULL;

			while (bl = findradius(bl, ent->s.origin, 1700))
			{
				if (!bl->inuse)
					continue;
				if (!bl->takedamage)
					continue;
				if (bl->solid != SOLID_BBOX)
					continue;
				if (!bl->waterlevel > 0)
					continue;

				T_Damage (bl, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 800, 800, 0, MOD_LIGHTNING_DIRECTDISCHARGE); // FIXME Mod
			}
		}
	}

	VectorSet(offset, 14, 3,  ent->viewheight-2);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	G_Spawn_Splash (TE_LASER_SPARKS, 1+((ent->client->ps.gunframe-5)*2), 0xe0e1e2e3, start, forward, start);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);


	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Thunderbolt (edict_t *ent)
{
	static int	pause_frames[]	= {21, 24};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 0};

	Weapon_Generic (ent, 4, 20, 24, 29, pause_frames, fire_frames, Weapon_Thunderbolt_Fire);

	if (ent->client->ps.gunframe == 6)
			gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/light_warmup.wav"), 1, ATTN_NORM, 0);

}



void Weapon_DKHammer_Temp_Smash (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	fire_melee (ent, start, forward, 100, 250, 200, 1, MOD_DKHAMMER); // FIXME Mod

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_SHOTGUN | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

}

void Weapon_DKHammer_Temp (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 3, 7, 0, 0, pause_frames, fire_frames, Weapon_DKHammer_Temp_Smash);

	// Time out.
	if (ent->client->dkhammer_framenum < level.framenum)
	{
		ent->client->pers.weapon = ent->client->temp_oldweapon;
		ent->client->temp_oldweapon = NULL;
		ent->client->weapon_sound = 0;
		ent->client->weaponstate = ent->client->old_weaponstate;
		ent->client->ps.gunframe = ent->client->old_gunframe;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Donkey Kong Hammer"))] = 0;
		return;
	}

	if (ent->client->ps.gunframe == 7)
		ent->client->ps.gunframe = 3;
}


void Weapon_Det_Fire (edict_t *ent)
{
	G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);

	T_Damage (ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, ent->health, 0, DAMAGE_NO_ARMOR|DAMAGE_ENERGY|DAMAGE_NO_PROTECTION, MOD_DETONATOR); // FIXME MOD

	if (is_quad)
		T_RadiusDamage(ent, ent, 800*3, NULL, 400*5, MOD_DETONATOR);
	else
		T_RadiusDamage(ent, ent, 200*3, NULL, 100*5, MOD_DETONATOR);

	
	ent->client->ps.gunframe++;
}

void Weapon_Det (edict_t *ent)
{
	static int	pause_frames[]	= {5, 8, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 2, 5, 8, 11, pause_frames, fire_frames, Weapon_Det_Fire);

	if (ent->client->ps.gunframe == 4)
		gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/button.wav"), 1, ATTN_NORM, 0);
}

// Portal gun
void fire_puddle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);

void Weapon_Portal_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	fire_puddle(ent, start, forward, 0, 1600);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Portal (edict_t *ent)
{
	static int	pause_frames[]	= {8, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 7, 11, 15, pause_frames, fire_frames, Weapon_Portal_Fire);
}