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


void weapon_grenade_fire(edict_t *ent, qboolean held, int gravity);

void Cmd_attack2b(edict_t * ent)
{
	//if (!ent->client->buttons & BUTTON_ATTACK2)
	//	return;
	ent->client->buttonsx &= ~BUTTON_ATTACK2;
	//ent->client->latched_buttons &= ~BUTTON_ATTACK2;
	if (ent->client->weapon_sound == gi.soundindex("weapons/BLASTSPLP.wav"))
	{
		ent->client->weapon_sound = 0;
		//ent->client->pers.secondary ^= SECONDARY_BLASTER;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/BLASTSPSP.wav"), 1, ATTN_IDLE, 0);
		int ix = ITEM_INDEX(FindItem("Charge"));
		ent->client->pers.inventory[ix] = 0;
		if (ent->client->pers.weapon_ext.scounter && ent->client->weaponstate == WEAPON_READY && ent->client->pers.weapon == FindItem("blaster"))
		{
			ent->client->ps.gunframe = 5;
			ent->client->weaponstate = WEAPON_FIRING;
			// start the animation
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
	}
	return;

	if (ent->client->pers.weapon == FindItem("shotgun"))
	{
		ent->client->pers.secondary &= ~SECONDARY_SHOTGUN;

	}
	else if (ent->client->pers.weapon == FindItem("railgun") && (ent->client->pers.secondary & SECONDARY_RAILGUN) && ent->client->weaponstate == WEAPON_FIRING)
	{
		ent->client->pers.secondary &= ~SECONDARY_RAILGUN;
	}
	return;
	if (ent->client->pers.weapon == FindItem("grenade launcher"))
	{
		ent->client->pers.secondary ^= SECONDARY_GRENADELAUNCHER;
	}
	else if (ent->client->pers.weapon == FindItem("rocket launcher"))
	{
		ent->client->pers.secondary ^= SECONDARY_ROCKETLAUNCHER;

	}
	else if (ent->client->pers.weapon == FindItem("grenades"))
	{
		ent->client->pers.secondary ^= SECONDARY_GRENADES;

	}
}

void Cmd_DualWielda(edict_t * ent)
{
	ent->client->buttonsx |= BUTTON_DUAL;
}

void Cmd_DualWieldb(edict_t * ent)
{

	ent->client->buttonsx &= ~BUTTON_DUAL;
}
void Cmd_Grapplea(edict_t * ent)
{
	ent->client->buttonsx |= BUTTON_GRAPPLE;
}

void Cmd_Grappleb(edict_t * ent)
{

	ent->client->buttonsx &= ~BUTTON_GRAPPLE;
}
void Cmd_attack3a(edict_t * ent)
{
	ent->client->buttonsx |= BUTTON_ATTACK3;
}

void Cmd_attack3b(edict_t * ent)
{
	ent->client->buttonsx &= ~BUTTON_ATTACK3;
}

void Cmd_attack2a(edict_t * ent)
{
	ent->client->buttonsx |= BUTTON_ATTACK2;
	//ent->client->latched_buttons |= BUTTON_ATTACK2;
	if (ent->client->pers.weapon == FindItem("grenade launcher"))
	{


		if (ent->client->pers.weapon_ext.mode)
		{

			ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_LAUNCHER_NORMAL;
			toggle_gl_ammo(ent);
			gi.cprintf(ent, PRINT_HIGH, "Loading now normal grenades.\n");


		}
		else
		{
			int ix = ITEM_INDEX(FindItem("cluster grenades"));
			if (!ent->client->pers.inventory[ix])
			{
				gi.cprintf(ent, PRINT_HIGH, "Out of item: cluster grenades\n");

				return;
			}

			ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER;
			toggle_gl_ammo(ent);
			gi.cprintf(ent, PRINT_HIGH, "Loading now cluster grenades.\n");


		}

		//gi.cprintf(ent, PRINT_HIGH, "gl sec attack\n");
	}
	else if (ent->client->pers.weapon == FindItem("chaingun"))
	{
		if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_CHAINGUN_ACCELERATING)
			ent->client->pers.weapon_ext.mode = WEAPON_MODE_CHAINGUN_DECCELERATING;
		else
			ent->client->pers.weapon_ext.mode = WEAPON_MODE_CHAINGUN_ACCELERATING;
	}
	return;
	char *msg;
	if (ent->client->pers.weapon == FindItem("blaster"))
	{
		ent->client->pers.secondary ^= SECONDARY_BLASTER;
	}
	else if (ent->client->pers.weapon == FindItem("shotgun"))
	{
		ent->client->pers.secondary ^= SECONDARY_SHOTGUN;
	}
	else if (ent->client->pers.weapon == FindItem("super shotgun") && !(ent->client->pers.secondary & SECONDARY_SUPERSHOTGUN) && ent->client->weaponstate != WEAPON_FIRING)
	{
		ent->client->pers.secondary ^= SECONDARY_SUPERSHOTGUN;
	}
	else if (ent->client->pers.weapon == FindItem("machinegun") && ent->client->weaponstate == WEAPON_READY)
	{
		ent->client->pers.secondary ^= SECONDARY_MACHINEGUN;
	}

	else if (ent->client->pers.weapon == FindItem("railgun") && !(ent->client->pers.secondary & SECONDARY_RAILGUN) && ent->client->weaponstate == WEAPON_READY)
	{
		ent->client->pers.secondary ^= SECONDARY_RAILGUN;
	}
	else if (ent->client->pers.weapon == FindItem("bfg10k") && ent->client->weaponstate == WEAPON_READY)
	{
		ent->client->pers.secondary ^= SECONDARY_BFG;
	}
	else if (ent->client->pers.weapon == FindItem("hyperblaster") && ent->client->weaponstate == WEAPON_READY)
	{

		ent->client->pers.secondary ^= SECONDARY_HYPERBLASTER;

	}
	return;
	if (ent->client->pers.weapon == FindItem("chaingun"))
	{
		ent->client->pers.secondary ^= SECONDARY_CHAINGUN;
		if (!ent->client->pers.secondary & SECONDARY_CHAINGUN)
		{
			ent->client->pers.secondary ^= SECONDARY_CHAINGUN_DECCELERATE;

		}
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak8 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack6 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
	}

	else if (ent->client->pers.weapon == FindItem("shotgun"))
	{
		ent->client->pers.secondary ^= SECONDARY_SHOTGUN;
		if (!(ent->client->pers.secondary & SECONDARY_SHOTGUN))
			msg = "SHOTGUN FULL AUTO MODE: OFF\n";
		else
			msg = "SHOTGUN FULL AUTO MODE: ON\n";
		gi.cprintf(ent, PRINT_HIGH, msg);
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/btnx.wav"), 0.5, ATTN_STATIC, 0);
	}
	else if (ent->client->pers.weapon == FindItem("rocket launcher"))
	{
		ent->client->pers.secondary ^= SECONDARY_ROCKETLAUNCHER;

		//ent->client->ps.gunframe = 5;


	}
	else if (ent->client->pers.weapon == FindItem("grenades"))
	{
		ent->client->pers.secondary ^= SECONDARY_GRENADES;

	}

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
void PlayerNoise_think(edict_t *ent)
{
	// this is a temporary bug fix
}
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
		if (item_mod)
		{
			if (who->client->pers.sile_health)
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
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;
		noise->think = PlayerNoise_think;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
		noise->think = PlayerNoise_think;
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

	VectorCopy(where, noise->s.origin);
	VectorSubtract(where, noise->maxs, noise->absmin);
	VectorAdd(where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity(noise);
}


qboolean Pickup_Weapon(edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;

	index = ITEM_INDEX(ent->item);

	if ((((int)(dmflags->value) & DF_WEAPONS_STAY)/* || coop->value*/)
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		// give them some ammo with it
		ammo = FindItem(ent->item->ammo);
		if ((int)dmflags->value & DF_INFINITE_AMMO)
			Add_Ammo(other, ammo, 1000);
		else
			Add_Ammo(other, ammo, ammo->quantity);

		if (!(ent->spawnflags & DROPPED_PLAYER_ITEM))
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn(ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}
	//	index = ITEM_INDEX(ent->client->pers.weapon->classname);
	//gi.bprintf(PRINT_HIGH, "PICKED UP A WEAPON!!!\n");

	if (other->client->pers.inventory[index] > 1)
	{
	//	gi.bprintf(PRINT_HIGH, "HAS 2 weapons, set the dual state, before was = %i\n", other->client->pers.weapon->dual);
		/*if (ent->item == FindItem("shotgun"))
			other->client->pers.weapon_ext.mag_sec |= GRENADE_SECOND;*/
	}
	/*else if(other->client->pers.inventory[index] == 1)
		if (ent->item == FindItem("shotgun"))
			other->client->pers.weapon_ext.mag_sec |= GRENADE_FIRST;*/
	if (other->client->pers.inventory[index] > 1 && other->client->pers.weapon->dual == WEAPON_DUAL_HASNT && other->client->pers.weapon == ent->item)
	{

		//gi.bprintf(PRINT_HIGH, "HAS 2 weapons, set the dual state\n");
		if(other->client->pers.weapon_ext.dual < WEAPON_DUAL_HAS)
			other->client->pers.weapon_ext.dual = WEAPON_DUAL_HAS;
	}

	if (other->client->pers.weapon != ent->item &&
		(other->client->pers.inventory[index] == 1) &&
		(!deathmatch->value || other->client->pers.weapon == FindItem("blaster")))
		other->client->newweapon = ent->item;

	return true;
}
void save_weapon_ext(edict_t *ent)
{
	/*	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_ACTIVATE_WEAPON)
		{
			gun_x->value = 15;
			gun_y->value = -15;
			gun_z->value = 5;
		}*/

	if (ent->client->pers.weapon == FindItem("bfg10k"))
	{
		//gi.bprintf(PRINT_HIGH, "BFG\n");
		ent->client->pers.weapon_ext_save.bfg10k = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("railgun"))
	{
		//gi.bprintf(PRINT_HIGH, "RAILGUN\n");
		ent->client->pers.weapon_ext_save.railgun = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("hyperblaster"))
	{
		//gi.bprintf(PRINT_HIGH, "HYPERBLASTER\n");
		ent->client->pers.weapon_ext_save.hyperblaster = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("rocket launcher"))
	{
		//gi.bprintf(PRINT_HIGH, "ROCKET LAUNCHER\n");
		ent->client->pers.weapon_ext_save.rocketlauncher = ent->client->pers.weapon_ext;
		gun_x->value = 0;
		gun_y->value = 5;
		gun_z->value = 4;

	}
	else if (ent->client->pers.weapon == FindItem("grenade launcher"))
	{
		//gi.bprintf(PRINT_HIGH, "GRENADE LAUNCHER\n");
		ent->client->pers.weapon_ext_save.grenadelauncher = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("grenades"))
	{
		//gi.bprintf(PRINT_HIGH, "GRENADES\n");
		ent->client->pers.weapon_ext_save.grenades = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("chaingun"))
	{
		//gi.bprintf(PRINT_HIGH, "CHAINGUN\n");
		ent->client->pers.weapon_ext_save.chaingun = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("machinegun"))
	{
		//gi.bprintf(PRINT_HIGH, "MACHINEGUN\n");
		ent->client->pers.weapon_ext_save.machinegun = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("super shotgun"))
	{
		//gi.bprintf(PRINT_HIGH, "SUPER SHOTGUN\n");
		ent->client->pers.weapon_ext_save.supershotgun = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("shotgun"))
	{
		//gi.bprintf(PRINT_HIGH, "SHOTGUN\n");
		ent->client->pers.weapon_ext_save.shotgun = ent->client->pers.weapon_ext;


	}
	else if (ent->client->pers.weapon == FindItem("blaster"))
	{
		//gi.bprintf(PRINT_HIGH, "BLASTER\n");
		ent->client->pers.weapon_ext_save.blaster = ent->client->pers.weapon_ext;


	}
}
void set_gunxyz(edict_t *ent)
{
	/*	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_ACTIVATE_WEAPON)
		{
			gun_x->value = 15;
			gun_y->value = -15;
			gun_z->value = 5;
		}*/
	
		if (ent->client->pers.weapon == FindItem("bfg10k"))
		{
			//gi.bprintf(PRINT_HIGH, "BFG\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.bfg10k;
			gun_x->value = 0;
			gun_y->value = 4;
			gun_z->value = 5;


		}
		else if (ent->client->pers.weapon == FindItem("railgun"))
		{
			//gi.bprintf(PRINT_HIGH, "RAILGUN\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.railgun;
			gun_x->value = 0;
			gun_y->value = 3;
			gun_z->value = 0;

		}
		else if (ent->client->pers.weapon == FindItem("hyperblaster"))
		{
			//gi.bprintf(PRINT_HIGH, "HYPERBLASTER\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.hyperblaster;
			gun_x->value = -1;
			gun_y->value = 5;
			gun_z->value = 0;

		}
		else if (ent->client->pers.weapon == FindItem("rocket launcher"))
		{
			//gi.bprintf(PRINT_HIGH, "ROCKET LAUNCHER\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.rocketlauncher;
			gun_x->value = 0;
			gun_y->value = 5;
			gun_z->value = 4;

		}
		else if (ent->client->pers.weapon == FindItem("grenade launcher"))
		{
			//gi.bprintf(PRINT_HIGH, "GRENADE LAUNCHER\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.grenadelauncher;
			gun_x->value = 0;
			gun_y->value = 5;
			gun_z->value = 4;

		}
		else if (ent->client->pers.weapon == FindItem("grenades"))
		{
			//gi.bprintf(PRINT_HIGH, "GRENADES\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.grenades;
			gun_x->value = 0;
			gun_y->value = 5;
			gun_z->value = 0;

		}
		else if (ent->client->pers.weapon == FindItem("chaingun"))
		{
			//gi.bprintf(PRINT_HIGH, "CHAINGUN\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.chaingun;
			gun_x->value = 0;
			gun_y->value = 5;
			gun_z->value = 4;

		}
		else if (ent->client->pers.weapon == FindItem("machinegun"))
		{
			//gi.bprintf(PRINT_HIGH, "MACHINEGUN\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.machinegun;
			gun_x->value = -3;
			gun_y->value = 5;
			gun_z->value = 0;

		}
		else if (ent->client->pers.weapon == FindItem("super shotgun"))
		{
			//gi.bprintf(PRINT_HIGH, "SUPER SHOTGUN\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.supershotgun;
			gun_x->value = -4;
			gun_y->value = 5;
			gun_z->value = 0;

		}
		else if (ent->client->pers.weapon == FindItem("shotgun"))
		{
			//gi.bprintf(PRINT_HIGH, "SHOTGUN\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.shotgun;
			gun_x->value = -5;
			gun_y->value = 5;
			gun_z->value = 0;

		}
		else if (ent->client->pers.weapon == FindItem("blaster"))
		{
			//gi.bprintf(PRINT_HIGH, "BLASTER\n");
			ent->client->pers.weapon_ext = ent->client->pers.weapon_ext_save.blaster;
			gun_x->value = -2;
			gun_y->value = 5;
			gun_z->value = 0;

		}
}
void check_pmodels(edict_t *ent)
{
	int city3 = 0;
	if (!strcmp(level.mapname, "city3"))
		city3 = 1;
	if (ent->health <= 0)
		return;

	int backup_model = ent->client->pers.weapon->weapmodel;
	int i;
	if (ent->client->pers.weapon == FindItem("hyperblaster") && !city3)
	{
		if (ent->client->pers.weapon_ext.dual < WEAPON_DUAL_ACTIVE)
			ent->client->pers.weapon->weapmodel = 9;
		else
			ent->client->pers.weapon->weapmodel = 15; //no more additional w_models

	}
	else if (ent->client->pers.weapon == FindItem("machinegun") && !city3)
	{
		if (ent->client->pers.weapon_ext.dual < WEAPON_DUAL_ACTIVE)
			ent->client->pers.weapon->weapmodel = 4;
		else
		ent->client->pers.weapon->weapmodel = 14;

	}
	else if (ent->client->pers.weapon == FindItem("shotgun"))
	{
		if (ent->client->pers.weapon_ext.dual < WEAPON_DUAL_ACTIVE && !city3)
			ent->client->pers.weapon->weapmodel = 2;
		else
			ent->client->pers.weapon->weapmodel = 13;

	}
	else if (ent->client->pers.weapon == FindItem("blaster") && !city3)
	{
		if (ent->client->pers.weapon_ext.dual < WEAPON_DUAL_ACTIVE)
			ent->client->pers.weapon->weapmodel = 1;
		else
			ent->client->pers.weapon->weapmodel = 12;

	}
	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}
	ent->client->pers.weapon->weapmodel = backup_model;
}
/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon(edict_t *ent)
{
	int i;
	
	save_weapon_ext(ent);

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire(ent, false, 0);
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
	check_pmodels(ent);
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
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
	ent->client->pers.secondary = 0;
	ent->client->pers.scount = 0;
	





	ent->client->anim_priority = ANIM_PAIN;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crpain1;
		ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
		ent->s.frame = FRAME_pain301;
		ent->client->anim_end = FRAME_pain304;

	}

	set_gunxyz(ent);

	if (ent->client->pers.weapon == FindItem("grenade launcher") && ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)
	{

		toggle_gl_ammo(ent);

	}
	gitem_t *index;
	index = FindItemByClassname(ent->client->pers.weapon->classname);

	int ix = ITEM_INDEX(index);
	//gi.bprintf(PRINT_HIGH, "ix = %i\n", ix);

	//gi.bprintf(PRINT_HIGH, "ITEM INDEX = %i", ix);
	if (ent->client->pers.inventory[ix] > 1)
		// (PRINT_HIGH, "HAS 2 weapons, set the dual state, before was = %i\n", ent->client->pers.weapon_ext.dual);
	if (ent->client->pers.inventory[ix] > 1 && ent->client->pers.weapon->dual == WEAPON_DUAL_HASNT && ent->client->pers.weapon_ext.dual < WEAPON_DUAL_HAS)
	{
		//gi.bprintf(PRINT_HIGH, "HAS 2 weapons, set the dual state\n");
		ent->client->pers.weapon_ext.dual = WEAPON_DUAL_HAS;
	}

	/*	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
		{
			ent->client->pers.weapon_ext.mag_sec = ent->client->pers.weapon->mag_sec * 2;
		}
		else
		{
			ent->client->pers.weapon_ext.mag_sec = ent->client->pers.weapon->mag_sec;
		}*/
	
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange(edict_t *ent)
{
	//gi.bprintf(PRINT_HIGH, "NoAmmoWeaponChange");
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))])
	{


		ent->client->newweapon = FindItem("railgun");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))])
	{

		ent->client->newweapon = FindItem("hyperblaster");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))])
	{

		ent->client->newweapon = FindItem("chaingun");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))])
	{

		ent->client->newweapon = FindItem("machinegun");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))])
	{

		ent->client->newweapon = FindItem("super shotgun");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))])
	{

		ent->client->newweapon = FindItem("shotgun");
		return;
	}

	ent->client->newweapon = FindItem("blaster");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon(edict_t *ent)
{
	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon(ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		if (item_mod->value)
			is_quad = (ent->client->pers.quad_health > 0 && ent->client->pers.items_activated & FL_QUAD);
		else
			is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots || ent->client->pers.sile_health)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink(ent);
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon(edict_t *ent, gitem_t *item)
{
	//if (ent->client->pers.weapon_ext.scounter)
	//	return;
	if (ent->client->weaponstate == (WEAPON_SWITCH_DUAL || WEAPON_SWITCH_MODE))
		return;
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;


	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO) && ent->client->pers.weapon != FindItem("blaster")) //this blaster check should not be done this way, but it accidentaly works
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		int ix;
		if (item == FindItem("grenade launcher") && !ent->client->pers.inventory[ammo_index])
		{
			if (ent->client->pers.weapon_ext_save.grenadelauncher.mode == WEAPON_MODE_GRENADE_LAUNCHER_NORMAL &&
				(ent->client->pers.inventory[ammo_index] != ITEM_INDEX(FindItem("Cluster Grenades")) &&
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] || ent->client->pers.inventory[ammo_index] != ITEM_INDEX(FindItem("Grenades")) &&
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Cluster Grenades"))]));
			{
				if (ent->client->pers.weapon_ext_save.grenadelauncher.mode == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)
					ent->client->pers.weapon_ext_save.grenadelauncher.mode = WEAPON_MODE_GRENADE_LAUNCHER_NORMAL;
				else
					ent->client->pers.weapon_ext_save.grenadelauncher.mode = WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER;
				toggle_gl_ammo(ent);
			}
		}
		else if (!ent->client->pers.inventory[ammo_index] && item != FindItem("blaster"))
		{

			gi.cprintf(ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity && item != FindItem("grenade launcher"))
		{
			gi.cprintf(ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}
	//if(ent->client->pers.weapon_ext_save.grenadelauncher.mode == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER);
		//toggle_gl_ammo(ent);
	// change to this weapon when down
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon(edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if (((item == ent->client->pers.weapon) || (item == ent->client->newweapon)) && (ent->client->pers.inventory[index] == 1))
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}
	//gi.bprintf(PRINT_HIGH, "DROPPING %s\n", item->classname);
	Drop_Item(ent, item);
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
qboolean weapon_extention(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST)
{
	if (ent->client->pers.items_activated & FL_HOLSTER_R || ent->client->pers.items_activated & FL_HOLSTER_E || ent->client->pers.items_activated & FL_ACTIVATING_R || ent->client->pers.items_activated & FL_ACTIVATING_E || ent->client->pers.items_activated & FL_DEACTIVATING_R || ent->client->pers.items_activated & FL_DEACTIVATING_E)
	{
		//gi.bprintf(PRINT_HIGH, "activated items = %i, gunframe = %i\n", ent->client->pers.items_activated, ent->client->ps.gunframe);

		if (ent->client->ps.gunframe < FRAME_DEACTIVATE_FIRST)
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if (ent->client->ps.gunframe < FRAME_DEACTIVATE_LAST)
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			if (ent->client->pers.items_activated & FL_HOLSTER_R)
			{
				if (ent->client->pers.items_activated & FL_REB)
				{
					ent->client->pers.items_activated |= FL_DEACTIVATING_R;
				}
				else
				{
					ent->client->pers.items_activated |= FL_ACTIVATING_R;
				}
				ent->client->pers.items_activated &= ~FL_HOLSTER_R;
			}
			else if (ent->client->pers.items_activated & FL_HOLSTER_E)
			{
				if (ent->client->pers.items_activated & FL_ENV)
				{
					ent->client->pers.items_activated |= FL_DEACTIVATING_E;
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/env_off.wav"), 1, ATTN_IDLE, 0);
				}
				else
				{
					ent->client->pers.items_activated |= FL_ACTIVATING_E;
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/env_on.wav"), 1, ATTN_IDLE, 0);
				}
				ent->client->pers.items_activated &= ~FL_HOLSTER_E;
			}
		}

		return true;
	}

	if (ent->client->kick)
	{

		//gi.bprintf(PRINT_HIGH, "kick = %i, gunframe = %i\n", ent->client->kick, ent->client->ps.gunframe);
		if (ent->client->kick == KICK_HOLSTER_START)
		{
			ent->client->ps.gunframe = FRAME_IDLE_LAST + 1;
			ent->client->kick = KICK_HOLSTER;

		}
		if (ent->client->kick == KICK_HOLSTER)
		{
			ent->client->ps.gunframe++;
			ent->client->ps.gunframe++;
			if (ent->client->ps.gunframe > FRAME_DEACTIVATE_LAST)
			{
				ent->client->ps.gunframe = 0;
				ent->client->kick = KICK_KICK;
				gi.sound(ent, CHAN_BODY, gi.soundindex("mutant/Mutatck1.wav"), 1, ATTN_NORM, 0);
				vmodel_backup = "models/weapons/v_kick/tris.md2";
				ent->client->ps.gunindex = gi.modelindex(vmodel_backup);

				vec3_t forward, right, start, offset, angles;
				VectorSet(offset, 8, 8, ent->viewheight);
				AngleVectors(ent->client->v_angle, forward, right, NULL);
				P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
				VectorScale(forward, -4, ent->client->kick_origin);

			}
		}
		else
			ent->client->ps.gunframe++;

		if (ent->client->kick >= KICK_KICK)
		{
			if (ent->client->ps.gunframe > 0 && ent->client->ps.gunframe < 3)
			{
				int damage = 15;
				int kick = 25;
				if (is_quad)
				{
					if (ent->client->pers.quad_health > damage * 0.2)
					{
						damage *= 4;
						kick *= 4;
						ent->client->pers.quad_health -= damage * 0.2;
					}
					else
					{
						damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
						kick *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
						ent->client->pers.quad_health = 0;
					}
				}
				kick_hit(ent, damage, kick);
			}
			if (ent->client->ps.gunframe == 4)
			{
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
				ent->client->kick = KICK_NONE;
				ent->client->weaponstate = WEAPON_ACTIVATING;
				ent->client->ps.gunframe = 0;
			}
		}



		return true;
	}
	//gi.bprintf(PRINT_HIGH, "pickup = %i, gunframe = %i\n", ent->client->pers.pickup, ent->client->ps.gunframe);

	if (ent->client->pers.pickup)
	{
		if (ent->client->pers.pickup == PICKUP_ATTEMPT)
		{
			if (ent->client->ps.gunframe != 0)
				ent->client->ps.gunframe--;
			return true;
		}
		else if (ent->client->pers.pickup == PICKUP_PICKINGUPSTART)
		{
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
			ent->client->pers.pickup = PICKUP_PICKINGUP;
			//gi.bprintf(PRINT_HIGH, "PICKINGUP: pickup = %i, gunframe = %i\n", ent->client->pers.pickup, ent->client->ps.gunframe);
			return true;

		}
		else if (ent->client->pers.pickup == PICKUP_PICKINGUP)
		{
			if (ent->client->ps.gunframe < FRAME_DEACTIVATE_LAST + 1)
				ent->client->ps.gunframe++;
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
				ent->client->pers.pickup = PICKUP_PICKINGUPLAST;
			else if (ent->client->ps.gunframe > FRAME_DEACTIVATE_LAST)
			{
				Cmd_pickup_depressed(ent);
			}
			return true;
		}
		else if (ent->client->pers.pickup == PICKUP_PICKEDUP)
		{
			if (ent->client->ps.gunframe < 5)
				ent->client->ps.gunframe++;

			return true;
		}
		else if (ent->client->pers.pickup == PICKUP_THROWING)
		{

			ent->client->ps.gunframe++;
			if (ent->client->ps.gunframe == 6)
			{
				ent->client->pers.weapon = ent->client->pers.lastweapon;
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
				//gi.bprintf(PRINT_HIGH, "THROW view model %s, last weapon view model %s\n", ent->client->pers.weapon->view_model, ent->client->pers.lastweapon->view_model);
				ent->client->ps.gunframe = 0;
				ent->client->pers.pickup = PICKUP_NONE;
				//client_cmd(ent, "-speed;");
			}
			return true;
			//	}
		}

	}
	return false;
}

void Weapon_Generic(edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void(*fire)(edict_t *ent))
{
	int		n;
	//gi.bprintf(PRINT_HIGH, "number is %i", ent->client->pers.weapon_ext.mode);

	//gi.bprintf(PRINT_HIGH, "pickup = %¿¿i\n", ent->client->pers.pickup);
	//if (ent->client->ps.gunframe > FRAME_DEACTIVATE_LAST)
	//gi.bprintf(PRINT_HIGH, "gunframe = %i dual = %i weaponstate = %i, scounter = %i, buttonsx = %i, mode = %i, mode2 = %i, magsec = %i\n", ent->client->ps.gunframe, ent->client->pers.weapon_ext.dual, ent->client->weaponstate, ent->client->pers.weapon_ext.scounter, ent->client->buttonsx, ent->client->pers.weapon_ext.mode, ent->client->pers.weapon_ext.mode2, ent->client->pers.weapon_ext.mag_sec);
	//else
	//gi.bprintf(PRINT_HIGH, "gunframe = %i, buttonsx = %i, ctf_grapplestate = %i\n", ent->client->ps.gunframe, ent->client->weaponstate, ent->client->ctf_grapplestate);
		//gi.bprintf(PRINT_HIGH, "gunframe = %i, buttonsx = %i, dual = %i, weaponstate = %i, FFF=%i, FIL=%i, scounter = %i, ctf_grapplestate = %i\n", ent->client->ps.gunframe, ent->client->buttonsx, ent->client->pers.weapon_ext.dual, ent->client->weaponstate, FRAME_FIRE_FIRST, FRAME_IDLE_LAST, ent->client->pers.weapon_ext.scounter, ent->client->ctf_grapplestate);
	if (weapon_extention(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST))
		return;
	
	if (ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{

		return;
	}
	if (ent->client->weaponstate == WEAPON_SWITCH_DUAL) //==================SWITCHING BETWEEN DUAL WIELD MODE====================//
	{

		if (!ent->client->ps.gunframe)
		{
			//gi.bprintf(PRINT_HIGH, "GUNFRAME = 0\n");
			if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_HAS)
			{
				//gi.bprintf(PRINT_HIGH, "GUNFRAME = 0, switching to second model\n");
				//gi.bprintf(PRINT_HIGH, "SECONDARY MACHINEGUN, scount = 0\n");
				//vmodel_backup = "models/weapons/v_machnd/tris_mod.md2";
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->second_view_model);
				ent->client->pers.weapon_ext.dual = WEAPON_DUAL_ACTIVE;
			}
			else if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_DEACTIVATING)
			{
				//gi.bprintf(PRINT_HIGH, "~SECONDARY MACHINEGUN, scount = 1\n");	
				//gi.bprintf(PRINT_HIGH, "GUNFRAME = 0, switching to original model\n");
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
				ent->client->pers.weapon_ext.dual = WEAPON_DUAL_HAS;
				check_pmodels(ent);
			}
		}
		else if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			//gi.bprintf(PRINT_HIGH, "GUNFRAME = FRAME_DEACTIVATE_LAST\n");
			ent->client->ps.gunframe = 0;
			return;
		}
		else if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			//gi.bprintf(PRINT_HIGH, "GUNFRAME = FRAME_ACTIVATE_LAST\n");
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			ent->client->weaponstate = WEAPON_READY;
			check_pmodels(ent);
		}
		ent->client->ps.gunframe++;
		return;
	}
	if (ent->client->weaponstate == WEAPON_SWITCH_MODE)		//==================SWITCHING BETWEEN WEAPON MODES====================//
	{
		if (FRAME_IDLE_LAST == 50)
		{
			if (!ent->client->pers.weapon_ext.scounter)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/rocket_chargeup.wav"), 1, ATTN_IDLE, 0);

			}
			int ix = ITEM_INDEX(FindItem("Cells"));
			if (ent->client->pers.inventory[ix] + ent->client->pers.weapon_ext.scounter < 50)
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				ent->client->pers.weapon_ext.scounter = 0;
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
					ent->pain_debounce_time = level.time + 1;
					gi.cprintf(ent, PRINT_HIGH, "No enough Cells for Heatbomb.\n");
				}
				return;
			}
			if (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1)
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			else
				ent->client->ps.gunframe = FRAME_IDLE_FIRST + 1;
			ent->client->pers.weapon_ext.scounter++;
			ent->client->pers.inventory[ix]--;
			if (ent->client->pers.weapon_ext.scounter == 50)
			{
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
			}
			return;
		}
		if (ent->client->pers.weapon == FindItem("super shotgun"))
		{
			if (ent->client->pers.weapon_ext.scounter >= SSHOTGUN_ALTFIRE_COUNT && ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			{
				//ent->client->weaponstate = WEAPON_ACTIVATING;
				ent->client->ps.gunframe = 0;
				return;
			}
			if (ent->client->pers.weapon_ext.scounter >= SSHOTGUN_ALTFIRE_COUNT && ent->client->pers.weapon_ext.scounter < 11 && ent->client->ps.gunframe < FRAME_DEACTIVATE_FIRST || ent->client->pers.weapon_ext.scounter == 13)
			{
				if (ent->client->pers.weapon_ext.scounter == 13)
					ent->client->pers.weapon_ext.scounter = SSHOTGUN_ALTFIRE_COUNT;

				ent->client->pers.weapon_ext.scounter++;
				if (ent->client->pers.weapon_ext.scounter == 8)
				{
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Sshotr1b.wav"), 1, ATTN_IDLE, 0);
				}
				if (ent->client->pers.weapon_ext.scounter == 10)
				{

					ent->client->pers.weapon_ext.scounter = 11;

				}
				return;
			}
			if (ent->client->pers.weapon_ext.scounter == 12 && ent->client->ps.gunframe >= FRAME_IDLE_FIRST)
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->pers.weapon_ext.scounter = 0;
				return;
			}
			if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
			{
				if (ent->client->pers.weapon_ext.scounter == 11)
				{
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Sshotf1c.wav"), 1, ATTN_IDLE, 0);
					ent->client->pers.weapon_ext.scounter = 12;
					ent->client->ps.gunframe = 12;
					return;
				}
				else
				{
					ent->client->weaponstate = WEAPON_READY;
					ent->client->ps.gunframe = FRAME_IDLE_FIRST;
					return;
				}
			}
			ent->client->ps.gunframe++;
		}

		if (ent->client->pers.weapon == FindItem("bfg10k") && ent->client->pers.inventory[ent->client->ammo_index] > 0)
		{
			//gi.bprintf(PRINT_HIGH, "MACHINEGUN DROPPING\n");
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 6)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/btnx.wav"), 0.5, ATTN_STATIC, 0);
			}
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 13)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/bfg_toggle.wav"), 1, ATTN_STATIC, 0);

			}
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 20)
			{
				if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_FAST)
				{
					//gi.bprintf(PRINT_HIGH, "===============BFG FAST================");
					//vmodel_backup = "models/weapons/v_bfg2/tris_mod.md2";
					ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->second_view_model);
				}
				else if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_NORMAL)
				{
					//gi.bprintf(PRINT_HIGH, "===============BFG NORMAL================");
					//gi.bprintf(PRINT_HIGH, "~SECONDARY MACHINEGUN, scount = 1\n");	
					ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
				}
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}
			ent->client->ps.gunframe++;
		}

		if (ent->client->pers.weapon == FindItem("hyperblaster") && ent->client->pers.inventory[ent->client->ammo_index] > 0)
		{
			int dual_frame_add = 0;
			if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
				dual_frame_add = 10;
			//gi.bprintf(PRINT_HIGH, "MACHINEGUN DROPPING\n");
			if (ent->client->ps.gunframe == (FRAME_DEACTIVATE_LAST + 3) && ent->client->pers.weapon_ext.dual != WEAPON_DUAL_ACTIVE || ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 7 + 19 || ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 7)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/btnx.wav"), 0.5, ATTN_STATIC, 0);
			}

			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 9 + dual_frame_add + 19)
			{
				ent->client->pers.weapon_ext.mode2++;
				if (ent->client->pers.weapon_ext.mode2 > WEAPON_MODE_HYPERBLASTER_SLOW)
				{
					ent->client->pers.weapon_ext.mode2 = WEAPON_MODE_HYPERBLASTER_NORMAL;
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hb_mode1.wav"), 1, ATTN_STATIC, 0);
				}
				else if (ent->client->pers.weapon_ext.mode2 == WEAPON_MODE_HYPERBLASTER_FAST)
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hb_mode2.wav"), 1, ATTN_STATIC, 0);
				else if (ent->client->pers.weapon_ext.mode2 == WEAPON_MODE_HYPERBLASTER_SLOW)
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hb_mode3.wav"), 1, ATTN_STATIC, 0);
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 9 + dual_frame_add)
			{
				ent->client->pers.weapon_ext.mode++;
				if (ent->client->pers.weapon_ext.mode > WEAPON_MODE_HYPERBLASTER_SLOW)
				{
					ent->client->pers.weapon_ext.mode = WEAPON_MODE_HYPERBLASTER_NORMAL;
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hb_mode1.wav"), 1, ATTN_STATIC, 0);
				}
				else if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_FAST)
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hb_mode2.wav"), 1, ATTN_STATIC, 0);
				else if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_SLOW)
					gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hb_mode3.wav"), 1, ATTN_STATIC, 0);
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			//gi.bprintf(PRINT_HIGH, "SECONDARY MACHINEGUN, scount = 0\n");




			ent->client->ps.gunframe++;
			return;
		}

	}
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		//gi.bprintf(PRINT_HIGH, "WEAPON_DROPPING, ammo = %i, FRAME_FIRE_LAST = %i, secondary = %i, scount = %i\n", ent->client->pers.inventory[ent->client->ammo_index], FRAME_FIRE_LAST, ent->client->pers.secondary & SECONDARY_MACHINEGUN, ent->client->pers.scount);




		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			//gi.bprintf(PRINT_HIGH, "OTHER DROPPING\n");
			ChangeWeapon(ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4 + 1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304 + 1;
				ent->client->anim_end = FRAME_pain301;

			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		//gi.bprintf(PRINT_HIGH, "WEAPON_ACTIVATING\n");

		if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->second_view_model);


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
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4 + 1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304 + 1;
				ent->client->anim_end = FRAME_pain301;

			}
		}
		return;
	}
	if (ent->client->weaponstate == WEAPON_CHARGINGUP)
	{
		if(!ent->client->pers.weapon_ext.scounter)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex(va("weapons/emp%i.wav", 1 + rand() % 4)), 1, ATTN_IDLE, 0);

		if(ent->client->pers.weapon_ext.scounter == 5)
			PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);
		ent->client->pers.weapon_ext.scounter++;
		if (ent->client->pers.weapon_ext.scounter == EMP_CHARGE_TIME)
		{
			
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;

				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
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
		return;
	}
	if (ent->client->weaponstate == WEAPON_RELOADING)
	{
		if (ent->client->ps.gunframe == 6)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Shotgr1b.wav"), 1, ATTN_IDLE, 0);
			ent->client->ps.gunframe++;
			return;
		}


		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST)
		{
			ent->client->weaponstate = WEAPON_READY;
			if (!(ent->client->pers.weapon_ext.mag_sec & GRENADE_FIRST))
				ent->client->pers.weapon_ext.mag_sec |= GRENADE_FIRST;
			return;
		}
		else if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 3)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Shotgr1b.wav"), 1, ATTN_IDLE, 0);
			ent->client->ps.gunframe++;
			return;
		}
		else if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 16)
		{
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Shotgr1b.wav"), 1, ATTN_IDLE, 0);
			ent->client->ps.gunframe++;
			return;
		}
		else if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST + 26)
		{
			ent->client->weaponstate = WEAPON_READY;
			if (!(ent->client->pers.weapon_ext.mag_sec & GRENADE_FIRST))
				ent->client->pers.weapon_ext.mag_sec |= GRENADE_FIRST;
			if (!(ent->client->pers.weapon_ext.mag_sec & GRENADE_SECOND))
				ent->client->pers.weapon_ext.mag_sec |= GRENADE_SECOND;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}


		ent->client->ps.gunframe++;
		return;
	}
	if (ent->client->weaponstate == WEAPON_READY)
	{
		//gi.bprintf(PRINT_HIGH, "FRAME FIRE LAST = %i, weapon_ext.scounter = %i\n", FRAME_FIRE_LAST, ent->client->pers.weapon_ext.scounter);
		if (ent->client->pers.weapon_ext.scounter && ent->client->pers.weapon == FindItem("super shotgun"))
		{
			ent->client->pers.weapon_ext.scounter = SSHOTGUN_ALTFIRE_COUNT;
			ent->client->weaponstate = WEAPON_SWITCH_MODE;
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

			//	gi.bprintf(PRINT_HIGH, "**********************************************************************\n");
			//	gi.bprintf(PRINT_HIGH, "*****************WEAPON READY, GOING TO SWITCH MODE*******************\n");
			//	gi.bprintf(PRINT_HIGH, "**********************************************************************\n");

			return;
		}
		if (ent->client->pers.weapon == FindItem("blaster"))
		{
			ent->client->pers.weapon_ext.loopstarted = 0;

		}
		if (ent->client->buttonsx & BUTTON_ATTACK2 || ent->client->buttonsx & BUTTON_ATTACK3)
		{
			//gi.bprintf(PRINT_HIGH, "WEAPON_READY\n");
			if (ent->client->pers.weapon == FindItem("shotgun"))
			{
				ent->client->pers.weapon_ext.scounter = 0;
				if (ent->client->buttonsx & BUTTON_ATTACK2)
				{
					if (!ent->client->pers.weapon_ext.mag_sec)
					{

						ent->client->weaponstate = WEAPON_RELOADING;

						if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
						{
							ent->client->ps.gunframe = 35;

						}
						else
							ent->client->ps.gunframe = 6;
						return;
					}
				}
			}
			else if (ent->client->pers.weapon == FindItem("hyperblaster"))
			{
				ent->client->weaponstate = WEAPON_SWITCH_MODE;
				if (ent->client->buttonsx & BUTTON_ATTACK3)
					ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST + 20;
				else
					ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST + 1;
				return;
			}
			if (ent->client->pers.weapon == FindItem("rocket launcher"))
			{
				int ix = ITEM_INDEX(FindItem("Cells"));
				if (ent->client->pers.inventory[ix] < 50)
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
						ent->pain_debounce_time = level.time + 1;
						gi.cprintf(ent, PRINT_HIGH, "No enough Cells for Super Rocket.\n");
					}

					return;
				}
				ent->client->ps.gunframe = FRAME_IDLE_FIRST + 1;
				ent->client->weaponstate = WEAPON_SWITCH_MODE;
				return;
			}
		}
		if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_HAS || ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE) //(FRAME_FIRE_LAST == LF_MACHINEGUN || FRAME_FIRE_LAST == LF_CHAINGUN)
		{
			//gi.bprintf(PRINT_HIGH, "WEAPON_READY, dual checks\n");

			if (ent->client->buttonsx & BUTTON_DUAL)
			{
				// gi.bprintf(PRINT_HIGH, "WEAPON_READY, pressed button, doing things, switching to dual wield weaponstate\n");
				ent->client->weaponstate = WEAPON_SWITCH_DUAL;
				if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
					ent->client->pers.weapon_ext.dual = WEAPON_DUAL_DEACTIVATING;
				ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
				
				return;
			}
		}
		else if (ent->client->pers.weapon == FindItem("bfg10k"))
		{
			//gi.bprintf(PRINT_HIGH, "WEAPON_READY, MACHINEGUN\n");
			if (ent->client->buttonsx & BUTTON_ATTACK2)
			{
				//gi.bprintf(PRINT_HIGH, "CHANGE OF WEAPON MODE, was = %i\n", ent->client->pers.weapon_ext.mode);

				ent->client->weaponstate = WEAPON_SWITCH_MODE;
				if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_FAST)
					ent->client->pers.weapon_ext.mode = WEAPON_MODE_BFG_NORMAL;
				else
					ent->client->pers.weapon_ext.mode = WEAPON_MODE_BFG_FAST;
				//gi.bprintf(PRINT_HIGH, "CHANGE OF WEAPON MODE is = %i\n", ent->client->pers.weapon_ext.mode);
				ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST + 1;
				return;
			}
		}

		if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK) || (ent->client->pers.weapon == FindItem("blaster") || ent->client->pers.weapon == FindItem("shotgun") && ent->client->pers.weapon_ext.mag_sec || ent->client->pers.weapon == FindItem("super shotgun") || ent->client->pers.weapon == FindItem("machinegun") || ent->client->pers.weapon == FindItem("chaingun") || ent->client->pers.weapon == FindItem("railgun")) && (ent->client->buttonsx & BUTTON_ATTACK2))
		{
			//gi.bprintf(PRINT_HIGH, "FIRE, magsec = %i\n", ent->client->pers.weapon_ext.mag_sec);
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (((!ent->client->ammo_index) ||
				(ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity) && ent->client->pers.weapon_ext.dual != WEAPON_DUAL_ACTIVE || ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity * 2 && ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE || (ent->client->pers.weapon == FindItem("blaster"))  || ent->client->pers.weapon == FindItem("machinegun")) && !(ent->client->pers.weapon == FindItem("shotgun") && (ent->client->buttonsx & BUTTON_ATTACK2)))
			{
				//gi.bprintf(PRINT_HIGH, "fire blaster n shit, frame fire last = %i\n", FRAME_FIRE_LAST);

				if (ent->client->pers.weapon == FindItem("blaster"))
				{
					/*if (ent->client->pers.weapon_ext.scounter == 666 || ent->client->pers.weapon_ext.scounter == 667)
						ent->client->pers.weapon_ext.scounter = 0;*/

					if (ent->client->buttonsx & BUTTON_ATTACK2)
					{

						int ix = ITEM_INDEX(FindItem("Charge"));
						if (ent->client->pers.inventory[ix] < 101)
							ent->client->pers.inventory[ix] = ent->client->pers.weapon_ext.scounter * 2;
						if (ent->client->pers.inventory[ix] > 100)
							ent->client->pers.inventory[ix] = 100;
						if (!ent->client->pers.weapon_ext.scounter)
						{
							gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/BLASTSPST.wav"), 1, ATTN_IDLE, 0);
							//ent->client->ps.gunframe = FRAME_IDLE_FIRST;

						}
						else
						{
							if (ent->client->pers.weapon_ext.scounter == 4)
							{
								ent->client->weapon_sound = gi.soundindex("weapons/BLASTSPLP.wav");
							}
							int num = 10;
							ent->client->ps.gunframe += num;
							if (ent->client->ps.gunframe > FRAME_IDLE_LAST - 20)
							{
								ent->client->ps.gunframe = FRAME_IDLE_FIRST + 5 + ent->client->ps.gunframe - (FRAME_IDLE_LAST - 20);
							}
							//gi.bprintf(PRINT_HIGH, "gunframe = %i\n", ent->client->ps.gunframe);
						}
						ent->client->pers.weapon_ext.scounter++;
						return;
					}


				}
				else if (ent->client->pers.weapon == FindItem("machinegun") && ent->client->buttonsx & BUTTON_ATTACK2)
				{
					int ix = ITEM_INDEX(FindItem("Cells"));
					if (ent->client->pers.inventory[ix] >= EMP_CELL_COST)
					{
						//gi.bprintf(PRINT_HIGH, "last shot = %f\n", ent->client->pers.weapon_ext.last_shot);
						if (ent->client->pers.weapon_ext.last_shot < level.time || ent->client->pers.weapon_ext.last_shot > level.time + EMP_RECHARGE_TIME)
						{
							ent->client->weaponstate = WEAPON_CHARGINGUP;
							ent->client->pers.weapon_ext.last_shot = level.time + EMP_RECHARGE_TIME;
							ent->client->pers.inventory[ix] -= EMP_CELL_COST;
						}
						else
							return;
					}
					else
					{
						if (level.time >= ent->pain_debounce_time)
						{
							gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
							ent->pain_debounce_time = level.time + 1;

						}
					}
					return;
				}

				if (ent->client->pers.weapon == FindItem("blaster") && ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
				{
					ent->client->ps.gunframe = 10;
					//gi.bprintf(PRINT_HIGH, "GOT DUAL WIELDED BLASTERS, GO TO LOOP ANIM\n");
				}
				else
					ent->client->ps.gunframe = FRAME_FIRE_FIRST;

				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
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

				if (ent->client->pers.weapon == FindItem("grenade launcher") && ent->client->pers.inventory[ent->client->ammo_index] <= 2 && 1 == 2)
				{
					if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)
						ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_LAUNCHER_NORMAL;
					else
						ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER;
					toggle_gl_ammo(ent);

				}

			}
			else if (ent->client->pers.weapon == FindItem("shotgun") && (ent->client->buttonsx & BUTTON_ATTACK2))
			{
				int ix = ITEM_INDEX(FindItem("Grenades"));
				if (ent->client->pers.inventory[ix] <= 0)
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
						ent->pain_debounce_time = level.time + 1;

					}
					//ent->client->ps.gunframe++;
					return;
				}
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;

				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
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
			else
			{


				if (ent->client->pers.weapon == FindItem("grenade launcher") && 1 == 2)
				{

					int ix = ITEM_INDEX(FindItem("Cluster Grenades"));
					int ix2 = ITEM_INDEX(FindItem("Grenades"));
					if (!ent->client->pers.inventory[ix] && !ent->client->pers.inventory[ix2])
					{
						//gi.bprintf(PRINT_HIGH, "SHOULD SWITCH WEAPONS, NO GRENADES OR CLUSTERS");
						if (level.time >= ent->pain_debounce_time)
						{
							gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
							ent->pain_debounce_time = level.time + 1;
						}
						NoAmmoWeaponChange(ent);
						return;
					}
					if (!ent->client->pers.inventory[ent->client->ammo_index])
					{
						if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)
							ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_LAUNCHER_NORMAL;
						else
							ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER;
						toggle_gl_ammo(ent);

					}
				}
				else
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
						ent->pain_debounce_time = level.time + 1;
					}
					NoAmmoWeaponChange(ent);
				}
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
						if (rand() & 15)
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
		if (ent->client->pers.weapon == FindItem("shotgun"))
		{
			if (ent->client->ps.gunframe == 6)
			{

				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;

				/*else if (ent->client->pers.weapon_ext.scounter)
				{
					ent->client->weaponstate = WEAPON_RELOADING;

						if (ent->client->pers.weapon_ext.mode != WEAPON_DUAL_ACTIVE)
							ent->client->ps.gunframe = 13;
						else
							ent->client->ps.gunframe = 34;
				}*/


			}
			/*	if (ent->client->pers.weapon_ext.scounter && ent->client->ps.gunframe == FRAME_FIRE_LAST)
				{
					ent->client->pers.weapon_ext.mag_sec = ent->client->pers.weapon->mag_sec;

					gi.bprintf(PRINT_HIGH, "shotgun reload");
				}*/

		}

		if (FRAME_IDLE_LAST == IL_BLASTER)
		{

		}
		if (ent->client->pers.weapon == FindItem("super shotgun"))
		{
			if (ent->client->ps.gunframe == 11 && (ent->client->pers.weapon_ext.scounter > 5 || !ent->client->pers.weapon_ext.scounter))
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Sshotf1c.wav"), 1, ATTN_IDLE, 0);

		}
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire(ent);
				break;
			}
		}
		if (ent->client->pers.weapon == FindItem("bfg10k"))
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_FIRST - 10 && ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_FAST)
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}
		}
		/*gi.bprintf(PRINT_HIGH, "FFL = %i, gunframe = %i, secondary = %i\n", FRAME_FIRE_LAST, ent->client->ps.gunframe, !(ent->client->pers.secondary & SECONDARY_SUPERSHOTGUN));
		if (FRAME_FIRE_LAST == 17 && ent->client->ps.gunframe == 7 && !(ent->client->pers.secondary & SECONDARY_SUPERSHOTGUN))
		{
			ent->client->ps.gunframe += 10;
		}
		else*/
		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST)
		{
			ent->client->weaponstate = WEAPON_READY;

		}
	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	600
#define GRENADE_MAXSPEED	1000

void weapon_grenade_fire(edict_t *ent, qboolean held, int gravity)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;
	vec3_t angles;

	radius = damage + 40;
	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2)
			{
				damage *= 4;
				ent->client->pers.quad_health -= damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
	}
		

	ent->client->pers.weapon_ext.mode = 0;
	VectorSet(offset, 8, 8, ent->viewheight - 8);
	VectorCopy(ent->client->v_angle, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	AngleVectors(angles, forward, right, NULL);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2(ent, start, forward, damage, speed, timer, radius, held, gravity);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;

	if (ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1 - 1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade(edict_t *ent)
{

	//weapon_extention(ent, 1, 15, 49, 51);
	int FRAME_ACTIVATE_LAST = 2;
	int FRAME_FIRE_LAST = 15;
	int FRAME_IDLE_LAST = 49;
	int FRAME_DEACTIVATE_LAST = 51;

	if (weapon_extention(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST))
		return;

	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon(ent);
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
		if ((ent->client->latched_buttons | ent->client->buttons) & (BUTTON_ATTACK) || ent->client->buttonsx & BUTTON_ATTACK2)
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
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange(ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand() & 15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->buttonsx & BUTTON_ATTACK2 && ent->client->buttons & BUTTON_ATTACK && ent->client->pers.weapon_ext.mode <= WEAPON_MODE_GRENADE_GRAVITY)
			ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_TRAP;
		else if (ent->client->buttonsx & BUTTON_ATTACK2 && !(ent->client->buttons & BUTTON_ATTACK) && !ent->client->pers.weapon_ext.mode)
			ent->client->pers.weapon_ext.mode = WEAPON_MODE_GRENADE_GRAVITY;

		if (ent->client->ps.gunframe == 5)
		{
			if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_GRAVITY)
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena2b.wav"), 1, ATTN_NORM, 0);
			else if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_GRENADE_TRAP)
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena3b.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
		}

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

				weapon_grenade_fire(ent, true, ent->client->pers.weapon_ext.mode);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & (BUTTON_ATTACK) || ent->client->buttonsx & BUTTON_ATTACK2)
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

			weapon_grenade_fire(ent, false, ent->client->pers.weapon_ext.mode);
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

void weapon_grenadelauncher_fire(edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t angles;
	int		damage = 120;
	float	radius;

	radius = damage + 40;
	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.4)
			{
				damage *= 4;
				ent->client->pers.quad_health -= damage * 0.4;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.4)) / (damage * 0.4));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
	}
	VectorSet(offset, 8, 8, ent->viewheight - 16);
	VectorCopy(ent->client->v_angle, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	AngleVectors(angles, forward, right, NULL);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;


	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_GRENADE | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
	//gi.bprintf(PRINT_HIGH, "queue1 = %i, queue2 = %i, mode = %i\n", ent->client->pers.weapon_ext.mag_queue1, ent->client->pers.weapon_ext.mag_queue2, ent->client->pers.weapon_ext.mode);

	int ix;
	//if(ent->client->pers.weapon_ext.mag_queue1 == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)

	//else if (ent->client->pers.weapon_ext.mag_queue1 == WEAPON_MODE_GRENADE_LAUNCHER_NORMAL)
	if (!((int)dmflags->value & DF_INFINITE_AMMO))
	{
		if (ent->client->pers.weapon_ext.mode == ent->client->pers.weapon_ext.mag_queue1)
			ent->client->pers.inventory[ent->client->ammo_index]--;
		else
		{

			if (ent->client->pers.weapon_ext.mag_queue1 == WEAPON_MODE_GRENADE_LAUNCHER_NORMAL)
			{
				ix = ITEM_INDEX(FindItem("Grenades"));
				if (ent->client->pers.inventory[ix])
				{
					ent->client->pers.inventory[ix]--;
					goto fire;
				}

				if (!ent->client->pers.inventory[ix])
					ent->client->pers.weapon_ext.mag_queue1 = WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER;

				ix = ITEM_INDEX(FindItem("Cluster grenades"));
				if (ent->client->pers.inventory[ix])
				{
					ent->client->pers.inventory[ix]--;
					goto fire;
				}
				if (!ent->client->pers.inventory[ix]) // no grenades and no cluster, change weapon
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
						ent->pain_debounce_time = level.time + 1;
					}
					NoAmmoWeaponChange(ent);
					return;
				}
			}
			else if (ent->client->pers.weapon_ext.mag_queue1 == WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER)
			{
				ix = ITEM_INDEX(FindItem("Cluster grenades"));
				if (ent->client->pers.inventory[ix])
				{
					ent->client->pers.inventory[ix]--;
					goto fire;
				}
				if (!ent->client->pers.inventory[ix])
					ent->client->pers.weapon_ext.mag_queue1 = WEAPON_MODE_GRENADE_LAUNCHER_NORMAL;

				ix = ITEM_INDEX(FindItem("Grenades"));
				if (ent->client->pers.inventory[ix])
				{
					ent->client->pers.inventory[ix]--;
					goto fire;
				}
				if (!ent->client->pers.inventory[ix]) // no grenades and no cluster, change weapon
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
						ent->pain_debounce_time = level.time + 1;
					}
					NoAmmoWeaponChange(ent);
					return;
				}

			}

		}
	}
fire:
	if (ent->client->pers.weapon_ext.mag_queue1 == WEAPON_MODE_GRENADE_LAUNCHER_NORMAL) //FIRE NORMAL
	{
		fire_grenade(ent, start, forward, damage, 1200, 2.5, radius);

		ent->client->pers.weapon_ext.mag_queue1 = ent->client->pers.weapon_ext.mag_queue2;

		//gi.bprintf(PRINT_HIGH, "normal, queue1 = %i, queue2 = %i, mode = %i\n", ent->client->pers.weapon_ext.mag_queue1, ent->client->pers.weapon_ext.mag_queue2, ent->client->pers.weapon_ext.mode);

	}
	else //FIRE CLUSTER
	{
		fire_grenade(ent, start, forward, damage, 1201, 2.5, radius);

		ix = ITEM_INDEX(FindItem("Cluster grenades"));
		if (ent->client->pers.inventory[ix] <= 2)
		{
			//gi.cprintf(ent, PRINT_HIGH, "Out of item: Cluster Grenades\n");
			ent->client->pers.weapon_ext.mag_queue1 = ent->client->pers.weapon_ext.mag_queue2;
			//ix = ITEM_INDEX(FindItem("Grenades"));
			//if (ent->client->pers.inventory[ix] > 1)
			//	ent->client->pers.weapon_ext.mag_queue2 = WEAPON_MODE_GRENADE_LAUNCHER_NORMAL;
		}
		else
		{

			ent->client->pers.weapon_ext.mag_queue1 = ent->client->pers.weapon_ext.mag_queue2;
			ent->client->pers.weapon_ext.mag_queue2 = WEAPON_MODE_GRENADE_LAUNCHER_CLUSTER;


		}
		//gi.bprintf(PRINT_HIGH, "cluster, queue1 = %i, queue2 = %i, mode = %i\n", ent->client->pers.weapon_ext.mag_queue1, ent->client->pers.weapon_ext.mag_queue2, ent->client->pers.weapon_ext.mode);

	}

	ent->client->pers.weapon_ext.mag_queue2 = ent->client->pers.weapon_ext.mode;
	//gi.bprintf(PRINT_HIGH, "end, queue1 = %i, queue2 = %i, mode = %i\n", ent->client->pers.weapon_ext.mag_queue1, ent->client->pers.weapon_ext.mag_queue2, ent->client->pers.weapon_ext.mode);



}

void Weapon_GrenadeLauncher(edict_t *ent)
{
	static int	pause_frames[] = { 34, 51, 59, 0 };
	static int	fire_frames[] = { 6, 0 };

	Weapon_Generic(ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire(edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if(item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.4)
			{
				damage *= 4;
				radius_damage *= 4;
				ent->client->pers.quad_health -= damage * 0.4;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.4)) / (damage * 0.4));
				radius_damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.4)) / (damage * 0.4));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
		radius_damage *= 4;
	}
	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 6, ent->viewheight - 12);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket(ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_ROCKET | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher(edict_t *ent)
{
	static int	pause_frames[] = { 25, 33, 42, 50, 0 };
	static int	fire_frames[] = { 5, 0 };

	Weapon_Generic(ent, 4, 12, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}



/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire(edict_t *ent, vec3_t g_offset, int damage, int type, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	int speed;
	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2)
			{
				damage *= 4;
				ent->client->pers.quad_health -= damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
	}

	vec3_t angles;
	VectorCopy(ent->client->v_angle, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	AngleVectors(angles, forward, right, NULL);
	VectorSet(offset, 24, 4, ent->viewheight - 8);
	VectorAdd(offset, g_offset, offset);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;
	if (ent->client->pers.weapon == FindItem("blaster"))
	{
		gi.WriteByte(svc_muzzleflash2);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ2_SOLDIER_BLASTER_1);
		gi.multicast(start, MULTICAST_PVS);
	}
	else
	{
		// send muzzle flash
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		if (type == BOLT_NOISY_HB || type == BOLT_NOISY2_HB || type == BOLT_SILENT_HB)
		{
			gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
			if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_SLOW)
				speed = 2000;
			else if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_FAST)
				speed = 1200;
			else
				speed = 1500;
		}

		else if (type == BOLT_NOISY || type == BOLT_SILENT)
		{
			gi.WriteByte(MZ_BLASTER | is_silenced);
			speed = 1200;
		}
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}
	fire_blaster(ent, start, forward, damage, speed, effect, type);




	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Blaster_Fire(edict_t *ent)
{
	int		damage;
	vec3_t	forward, right, offset, angles;
	vec3_t	start, start2;
	int speed;
	int type;
	int dual_counter = 0;
	float s_offset = 0;

	int volume = 1;
	if (is_silenced)
		volume = 0.1;

	int		hand = ent->client->pers.hand;
	if (ent->client->ps.gunframe == 5 && !ent->client->pers.weapon_ext.scounter && ent->client->pers.weapon_ext.dual != WEAPON_DUAL_ACTIVE)
		ent->client->ps.gunframe = 6;

	if (ent->client->ps.gunframe == 9 && !ent->client->pers.weapon_ext.scounter)
	{
		ent->client->ps.gunframe = 18;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}



	if (ent->client->ps.gunframe == 12 || ent->client->ps.gunframe == 14 || ent->client->ps.gunframe == 16 || ent->client->pers.weapon_ext.loopstarted && ent->client->ps.gunframe == 10)
	{
		if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE && !ent->client->pers.weapon_ext.scounter)
		{
			//if (ent->client->ps.gunframe == 5)
			//	ent->client->ps.gunframe = 9;



			if (!(ent->client->buttons & BUTTON_ATTACK))
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = 18;
				return;
			}
			else
			{
				ent->client->ps.gunframe++;
				return;
			}


		}


	}





	if (deathmatch->value)
		damage = 15;
	else
		damage = 10;
	if (ent->client->ps.gunframe == 11 || ent->client->ps.gunframe == 15)
	{
		if (hand)
			ent->client->pers.hand = 0;
		else
			ent->client->pers.hand = 1;

	}


	ent->client->pers.weapon_ext.loopstarted = 1;
fire:
	/*****************************************from Blaster_fire function**********************/


	if (dual_counter && ent->client->pers.weapon_ext.scounter)
	{
		s_offset = 0.01 + (random() / 50);
	}
	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2)
			{
				damage *= 4;
				ent->client->pers.quad_health -= damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
	}
	VectorCopy(ent->client->v_angle, angles);
	AngleVectors(angles, forward, right, NULL);

	offset[0] = 24;
	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
		offset[1] = 6;
	else
		offset[1] = 4;
	offset[2] = ent->viewheight - 8;

	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] += -1;

	// send muzzle flash

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("soldier/Solatck2.wav"), volume, ATTN_NORM, s_offset);

	fire_blaster(ent, start, forward, damage, 1200, EF_BLASTER, BOLT_NOISY);
	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
	{
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

	if (/*ent->client->pers.inventory[ent->client->ammo_index] &&*/ ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE && ent->client->pers.weapon_ext.scounter && !dual_counter)
	{

		if (ent->client->pers.hand == 0)
			ent->client->pers.hand = 1;
		else
			ent->client->pers.hand = 0;

		++dual_counter;
		VectorCopy(start, start2);
		// start the animation

		goto fire;
	}
	if (dual_counter)
	{
		if (ent->client->pers.hand == 0)
			ent->client->pers.hand = 1;
		else
			ent->client->pers.hand = 0;

		
	}
	if (dual_counter && ent->client->pers.weapon_ext.scounter)
	{
		VectorAdd(start, start2, start);
		VectorScale(start, 0.5, start);

	}
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_NUKE2);
	gi.multicast(start, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	///	Blaster_Fire(ent, vec3_origin, damage, BOLT_NOISY, EF_BLASTER);
	if (ent->client->ps.gunframe == 11 || ent->client->ps.gunframe == 15)
	{
		ent->client->pers.hand = hand;
	}
	if (ent->client->ps.gunframe == 17)
	{
		ent->client->ps.gunframe = 10;
		return;
	}
	ent->client->ps.gunframe++;
	ent->client->pers.weapon_ext.scounter = 0;

}

void Weapon_Blaster(edict_t *ent)
{
	static int	pause_frames[] = { 28, 41, 0 };

	//static int	fire_frames[] = { 5, 0};
	static int	fire_frames[] = { 5, 9, 10, 11, 13, 15, 16, 17, 0 };
	Weapon_Generic(ent, 4, 17, 61, 64, pause_frames, fire_frames, Weapon_Blaster_Fire);
}


void Weapon_HyperBlaster_Fire(edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;
	vec3_t	forward, right;
	vec3_t	start;
	int speed;
	int type;
	int scounter = 0;
	int		hand = ent->client->pers.hand;
	int dual_counter = 0;
	float s_offset = 0;
	int fired = 0;
	int ammo_spent = 0;
	vec3_t start2;
	int current_mode = ent->client->pers.weapon_ext.mode;

	int volume = 1;
	if (is_silenced)
		volume = 0.1;

	if (!ent->client->weapon_sound)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Hyprbu1a.wav"), 1, ATTN_IDLE, 0);

	if (ent->client->pers.weapon_ext.mode)
		ent->client->weapon_sound = gi.soundindex("weapons/hyprbl2a.wav");
	else
		ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");



	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		scounter = 0;
	}
	else
	{
		if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_SLOW)
		{

			if (ent->client->pers.inventory[ent->client->ammo_index] < 2)
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange(ent);
				goto skip;
			}
		}
		else
		{
			if (!ent->client->pers.inventory[ent->client->ammo_index])
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange(ent);
				goto skip;
			}
		}

		if (!((int)dmflags->value & DF_INFINITE_AMMO) && ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_SLOW)
			ent->client->pers.inventory[ent->client->ammo_index]--;
		++ammo_spent;

		if (!((int)dmflags->value & DF_INFINITE_AMMO) && ent->client->pers.weapon_ext.mode2 && ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
		{
			ent->client->pers.inventory[ent->client->ammo_index]--;
			++ammo_spent;
		}
	fire:
		if (!ent->client->pers.inventory[ent->client->ammo_index])
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange(ent);
		}
		else
		{
			if (ent->client->pers.weapon_ext.scounter && current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
			{

				goto skip;
			}

			rotation = (ent->client->ps.gunframe - 5 + scounter) * 2 * M_PI / 6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if (current_mode == WEAPON_MODE_HYPERBLASTER_NORMAL)
			{
				effect = EF_HYPERBLASTER;
				type = BOLT_NOISY_HB;
				ent->client->kick_angles[0] += -1;
			}
			else if (current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
			{
				effect = EF_HYPERBLASTER;
				type = BOLT_NOISY2_HB_SLOW;
				ent->client->kick_angles[0] += -1;
			}
			else if (current_mode == WEAPON_MODE_HYPERBLASTER_FAST)
			{
				effect = EF_BLASTER;
				if (scounter)
				{
					type = BOLT_SILENT_HB;
					ent->client->kick_angles[0] += -0.1;
				}
				else
				{
					ent->client->kick_angles[0] += -1;
					type = BOLT_NOISY_HB;
				}


			}

			/*if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9) && !(ent->client->pers.secondary & SECONDARY_HYPERBLASTER))
				effect = EF_HYPERBLASTER;
			else
				effect = EF_BLASTER;*/
			if (current_mode == WEAPON_MODE_HYPERBLASTER_NORMAL)
			{
				if (deathmatch->value)
					damage = 15;
				else
					damage = 20;
			}
			else if (current_mode == WEAPON_MODE_HYPERBLASTER_FAST)
			{
				if (deathmatch->value)
					damage = 15;
				else
					damage = 10;
			}
			else if (current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
			{
				if (deathmatch->value)
					damage = 25;
				else
					damage = 30;
				//ent->client->pers.weapon_ext.scounter++;
				gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Hyprbf2a.wav"), volume, ATTN_NORM, 0);

			}
			/*****************************************from Blaster_fire function**********************/


			if (item_mod->value)
			{
				if (is_quad)
				{
					if (ent->client->pers.quad_health > damage * 0.2)
					{
						damage *= 4;
						ent->client->pers.quad_health -= damage * 0.2;
					}
					else
					{
						damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
						ent->client->pers.quad_health = 0;
					}
				}
			}
			else
			{
				damage *= 4;
			}
			vec3_t angles;
			VectorCopy(ent->client->v_angle, angles);
			angles[PITCH] += ent->client->v_dmg_pitch;
			angles[ROLL] += ent->client->v_dmg_roll;
			angles[YAW] += ent->client->v_dmg_yaw;
			AngleVectors(angles, forward, right, NULL);

			offset[0] += 24;

			if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
				offset[1] += 8;
			else
				offset[1] += 4;
			offset[2] += ent->viewheight - 10;

			P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

			VectorScale(forward, -2, ent->client->kick_origin);

			if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE && current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
				ent->client->kick_angles[0] += -1;

			if (!dual_counter)
				VectorCopy(start, start2);
			if (current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
				speed = 2000;
			else if (current_mode == WEAPON_MODE_HYPERBLASTER_FAST)
				speed = 1200;
			else
				speed = 1500;

			fire_blaster(ent, start, forward, damage, speed, effect, type);

			if (!fired)
				fired = 1;
			if (current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
				fired++;
			s_offset = 0.00666 + (scounter / 133) + (random() / 66);
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("soldier/Solatck2.wav"), 1, ATTN_NORM, s_offset);

			/***********************************************************************************/
			//Blaster_Fire(ent, offset, damage, type, effect);


			if (!((int)dmflags->value & DF_INFINITE_AMMO))
			{
				if (current_mode == WEAPON_MODE_HYPERBLASTER_SLOW)
				{
					if (ammo_spent)
						ammo_spent--;
					else
						ent->client->pers.inventory[ent->client->ammo_index]--;
				}
				else if(ammo_spent < 2)
					ent->client->pers.inventory[ent->client->ammo_index]--;

			}


		}

		//``
	skip:
		if (current_mode == WEAPON_MODE_HYPERBLASTER_FAST)
		{

			scounter++;
			if (scounter < 3)
			{
				goto fire;
			}
		}
		if (!dual_counter)
		{

			if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
			{
				scounter = 0;
				dual_counter = 1;
				reverse_hand(ent);
				current_mode = ent->client->pers.weapon_ext.mode2;
				goto fire;
			}
		}
		//ent->client->ps.gunframe += 2;

	//else

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}
	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_IDLE, 0);
		ent->client->weapon_sound = 0;
	}

	if (ent->client->pers.weapon_ext.dual != WEAPON_DUAL_ACTIVE && ent->client->pers.weapon_ext.mode != WEAPON_MODE_HYPERBLASTER_FAST && fired)
	{
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
	}
	else if (fired)
	{
		VectorAdd(start, start2, start);
		VectorScale(start, 0.5, start);
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_NUKE2);
		gi.multicast(start, MULTICAST_PVS);
	}

	if (ent->client->pers.weapon_ext.scounter && (ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_SLOW || ent->client->pers.weapon_ext.mode2 == WEAPON_MODE_HYPERBLASTER_SLOW && ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE))
		ent->client->pers.weapon_ext.scounter = 0;

	if (fired)
	{
		if ((ent->client->pers.weapon_ext.mode == WEAPON_MODE_HYPERBLASTER_SLOW || ent->client->pers.weapon_ext.mode2 == WEAPON_MODE_HYPERBLASTER_SLOW) && fired > 1)
			ent->client->pers.weapon_ext.scounter = 1;

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
	ent->client->pers.hand = hand;

}

void Weapon_HyperBlaster(edict_t *ent)
{
	static int	pause_frames[] = { 0 };
	static int	fire_frames[] = { 6, 7, 8, 9, 10, 11, 0 };

	Weapon_Generic(ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire(edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;
	float f_offset;
	int dual_counter = 0;
	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2)
			{
				damage *= 4;
				kick *= 4;
				ent->client->pers.quad_health -= damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				kick *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
		kick *= 4;
	}
emp:

	   // gi.bprintf(PRINT_HIGH, "sec attack machinegun scounter = %i", ent->client->pers.weapon_ext.scounter);

		//if (ent->client->pers.weapon_ext.scounter == 1)
		if (ent->client->pers.weapon_ext.scounter == EMP_CHARGE_TIME)
		{
			
			VectorCopy(ent->client->v_angle, angles);
			angles[PITCH] += ent->client->v_dmg_pitch;
			angles[ROLL] += ent->client->v_dmg_roll;
			angles[YAW] += ent->client->v_dmg_yaw;
			AngleVectors(angles, forward, right, NULL);

			
			VectorSet(offset, 0, 6, ent->viewheight - 8);
			P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
		

			fire_emp(ent, start, forward, damage);
			ent->client->pers.weapon_ext.scounter = 0;
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/emp_f.wav"), 1, ATTN_NORM, 0);


		}

	
	
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
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange(ent);
		return;
	}



fire:

	for (i = 1; i < 3; i++)
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
	int spreadh;
	int spreadv;
	// get start / end positions
	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
	{
		spreadh = DEFAULT_BULLET_HSPREAD * 2;
		spreadv = DEFAULT_BULLET_VSPREAD * 1.5;
	}
	else
	{
		spreadh = DEFAULT_BULLET_HSPREAD;
		spreadv = DEFAULT_BULLET_VSPREAD;

	}


	VectorCopy(ent->client->v_angle, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	AngleVectors(angles, forward, right, NULL);

	VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors(angles, forward, right, NULL);
	VectorSet(offset, 0, 6, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet(ent, start, forward, damage, kick, spreadh, spreadv, MOD_MACHINEGUN);




	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;

	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE && ent->client->pers.inventory[ent->client->ammo_index] > 1 && !dual_counter)
	{
		dual_counter++;

		if (ent->client->pers.hand == 0)
			ent->client->pers.hand = 1;
		else
			ent->client->pers.hand = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex(va("weapons/machgf%ib.wav", (rand() % 5) + 1)), 1, ATTN_NORM, 0);
		PlayerNoise(ent, start, PNOISE_WEAPON);
		f_offset = 0.01 + (random() / 50);
		goto fire;


	}


	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int)(random() + 0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int)(random() + 0.25);
		ent->client->anim_end = FRAME_attack8;
	}

	if (dual_counter)
	{
		if (ent->client->pers.hand == 0)
			ent->client->pers.hand = 1;
		else
			ent->client->pers.hand = 0;
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_NUKE2);
		gi.multicast(start, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_MACHINEGUN | is_silenced);
		gi.multicast(start, MULTICAST_PVS);
	}

}

void Weapon_Machinegun(edict_t *ent)
{
	static int	pause_frames[] = { 23, 45, 0 };
	static int	fire_frames[] = { 4, 5, 0 };

	Weapon_Generic(ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire(edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset, angles;
	int			damage;
	int			kick = 2;
	int counter = 0;
	int spreadh;
	int spreadv;

	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
	{
		spreadh = DEFAULT_BULLET_HSPREAD * 1.25;
		spreadv = DEFAULT_BULLET_VSPREAD * 1.1;
	}
	else
	{
		spreadh = DEFAULT_BULLET_HSPREAD;
		spreadv = DEFAULT_BULLET_VSPREAD;

	}

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;



	if (ent->client->ps.gunframe == 5)
	{
		if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a_3.wav"), 1, ATTN_IDLE, 0);
		else
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	}
	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK) && ent->client->pers.weapon_ext.mode == WEAPON_MODE_CHAINGUN_DECCELERATING)
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && ((ent->client->buttons & BUTTON_ATTACK) || ent->client->pers.weapon_ext.mode == WEAPON_MODE_CHAINGUN_ACCELERATING)
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
		if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a_3.wav"), 1, ATTN_IDLE, 0);
		else
			gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
			ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a_3.wav");
		else
			ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");

	}

	if (ent->client->buttons & BUTTON_ATTACK)
	{
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
	}
	else
	{
		if (ent->client->anim_priority == ANIM_BASIC)
		{
			if (ent->s.frame >= FRAME_crstnd01 && ent->s.frame <= FRAME_crstnd19 || (ent->client->ps.pmove.pm_flags & PMF_DUCKED) && VectorLength(ent->velocity) == 0)
				ent->s.frame = FRAME_attack1;
			else if (ent->s.frame >= FRAME_stand01 && ent->s.frame <= FRAME_stand40 || VectorLength(ent->velocity) == 0)
				ent->s.frame = FRAME_attack1;

		}
		for (i = 0; i < 3; i++)
		{
			ent->client->kick_origin[i] = crandom() * 0.0875 + ((random() * (i + counter)) / 10);
			ent->client->kick_angles[i] = crandom() * 0.175 + ((random() * (i + counter)) / 10);
		}
		PlayerNoise(ent, start, PNOISE_SELF);
		return;
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


	if (item_mod->value && shots)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2 * shots)
			{
				damage *= 4;
				kick *= 4;
				ent->client->pers.quad_health -= damage * 0.2 * shots;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2 * shots)) / (damage * 0.2 * shots));
				kick *= 4 - ((ent->client->pers.quad_health - (damage * 0.2 * shots)) / (damage * 0.2 * shots));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
		kick *= 4;
	}


	for (i = 0; i < 3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35 + ((random() * (i + counter)) / 10);
		ent->client->kick_angles[i] = crandom() * 0.7 + ((random() * (i + counter)) / 10);
	}

fire:
	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange(ent);
		return;
	}
	for (i = 0; i < shots; i++)
	{
		// get start / end positions
		VectorCopy(ent->client->v_angle, angles);
		angles[PITCH] += ent->client->v_dmg_pitch;
		angles[ROLL] += ent->client->v_dmg_roll;
		angles[YAW] += ent->client->v_dmg_yaw;
		AngleVectors(angles, forward, right, NULL);
		r = 7 + crandom() * 4;
		u = crandom() * 4;
		VectorSet(offset, 0, r, u + ent->viewheight - 8);
		P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet(ent, start, forward, damage, kick, spreadh, spreadv, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast(start, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE && counter < 3)
	{
		counter++;
		goto fire;
	}
}


void Weapon_Chaingun(edict_t *ent)
{
	static int	pause_frames[] = { 38, 43, 51, 61, 0 };
	static int	fire_frames[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0 };

	Weapon_Generic(ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire(edict_t *ent)
{
	vec3_t		start, start2;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t angles;
	int			damage = 4;
	int			kick = 8;
	int grenade_damage = 100;
	if (deathmatch->value)
		grenade_damage = 75;

	int volume = 1;
	if (is_silenced)
		volume = 0.1;

	float s_offset = 0;
	int fired_grenade = 0;
	int fired = 0;
	if (ent->client->ps.gunframe == 5)
	{
		ent->client->ps.gunframe++;
		return;
	}
	int dual_counter = 0;

fire:
	VectorCopy(ent->client->v_angle, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	if (ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE)
	{
		angles[0] += 1 + (crandom() * 5);
		angles[1] += 1 + (crandom() * 5);
		angles[2] += 1 + (crandom() * 5);
	}
	AngleVectors(angles, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 6, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > grenade_damage * 0.2)
			{
				damage *= 4;
				kick *= 4;
				ent->client->pers.quad_health -= grenade_damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (grenade_damage * 0.2)) / (grenade_damage * 0.2));
				kick *= 4 - ((ent->client->pers.quad_health - (grenade_damage * 0.2)) / (grenade_damage * 0.2));
				grenade_damage *= 4 - ((ent->client->pers.quad_health - (grenade_damage * 0.2)) / (grenade_damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
		grenade_damage *= 4;
		kick *= 4;
	}

	if (ent->client->buttonsx & BUTTON_ATTACK2)
	{
		if (!ent->client->pers.weapon_ext.mag_sec)
		{

			/*ent->client->weaponstate = WEAPON_RELOADING;

			if (ent->client->pers.weapon_ext.mode != WEAPON_DUAL_ACTIVE)
				ent->client->ps.gunframe = 13;
			else
				ent->client->ps.gunframe = 34;
				*/
			//gi.bprintf(PRINT_HIGH, "!ent->client->pers.weapon_ext.mag_sec\n");

			goto skip;
		}
		else
		{
			if (!dual_counter && !(ent->client->pers.weapon_ext.mag_sec & GRENADE_FIRST))
			{
				//gi.bprintf(PRINT_HIGH, "!dual_counter && !(ent->client->pers.weapon_ext.mag_sec & GRENADE_FIRST), %i, %i\n", dual_counter, ent->client->pers.weapon_ext.mag_sec & GRENADE_FIRST);
				goto skip;
			}
				

			int ix = ITEM_INDEX(FindItem("Grenades"));
			if (ent->client->pers.inventory[ix] <= 0)
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
					ent->pain_debounce_time = level.time + 1;

				}
				//gi.bprintf(PRINT_HIGH, "ent->client->pers.inventory[ix] <= 0\n");

				goto skip;
			}
			
				



			fired_grenade = 1;
			fire_shotgun_grenade(ent, start, forward, grenade_damage, SHOTGUN_GRENADE_SPEED);
			fired = 1;
			//gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Shotgr1b.wav"), 1, ATTN_IDLE, 0.1 + s_offset);

			ent->client->pers.weapon_ext.scounter = 1;
			if (ent->client->pers.weapon_ext.mag_sec & GRENADE_FIRST)
				ent->client->pers.weapon_ext.mag_sec &= ~GRENADE_FIRST;
			else if (ent->client->pers.weapon_ext.mag_sec & GRENADE_SECOND)
				ent->client->pers.weapon_ext.mag_sec &= ~GRENADE_SECOND;
			ent->client->pers.inventory[ix]--;
		}
	}
	else
	{
		ent->client->pers.weapon_ext.scounter = 0;
		if (deathmatch->value)
			fire_shotgun(ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
		else
			fire_shotgun(ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);
		fired = 1;
	}
	// send muzzle flash
	/*gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ2_CARRIER_MACHINEGUN_R2 | is_silenced);
	gi.multicast(start, MULTICAST_PVS);*/

	if (dual_counter)
	{
		VectorAdd(start, start2, start);
		VectorScale(start, 0.5, start);
		s_offset = 0.01 + (random() / 50);
	}

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/Shotgf1b.wav"), volume, ATTN_NORM, s_offset);
	if (!fired_grenade)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/Shotgr2b.wav"), 1, ATTN_IDLE, s_offset);






	if (!((int)dmflags->value & DF_INFINITE_AMMO) && !(ent->client->buttonsx & BUTTON_ATTACK2))
		ent->client->pers.inventory[ent->client->ammo_index]--;
skip:
	if ((ent->client->pers.weapon_ext.mag_sec || ent->client->pers.inventory[ent->client->ammo_index]) && ent->client->pers.weapon_ext.dual == WEAPON_DUAL_ACTIVE && !dual_counter)
	{

		if (ent->client->pers.hand == 0)
			ent->client->pers.hand = 1;
		else
			ent->client->pers.hand = 0;
		++dual_counter;
		VectorCopy(start, start2);
		goto fire;
	}
	ent->client->ps.gunframe++;
	if (fired)
	{
		if (dual_counter)
		{
			if (ent->client->pers.hand == 0)
				ent->client->pers.hand = 1;
			else
				ent->client->pers.hand = 0;
		}
		PlayerNoise(ent, start, PNOISE_WEAPON);

		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_NUKE2);
		gi.multicast(start, MULTICAST_PVS);
	}
}

void Weapon_Shotgun(edict_t *ent)
{
	static int	pause_frames[] = { 17, 23, 29, 0 };
	static int	fire_frames[] = { 3, 0 };

	Weapon_Generic(ent, 2, 13, 31, 34, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire(edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

	//gi.bprintf(PRINT_HIGH, "gunframe = %i, scount = %i\n", ent->client->ps.gunframe, ent->client->pers.scount);
	if (ent->client->ps.gunframe == 12 || ent->client->ps.gunframe == 7)
	{

		if (ent->client->buttonsx & BUTTON_ATTACK2 || ent->client->pers.weapon_ext.scounter)
		{
			//gi.bprintf(PRINT_HIGH, "SECONDARY\n");
			if (ent->client->ps.gunframe == 7)
			{
				//gi.bprintf(PRINT_HIGH, "gunframe = 7\n");
				if (ent->client->pers.weapon_ext.scounter < SSHOTGUN_ALTFIRE_COUNT)
				{
					//gi.bprintf(PRINT_HIGH, "scount < 3\n");
					ent->client->ps.gunframe = 7;
					ent->client->pers.weapon_ext.scounter++;
				}


			}
			if (ent->client->pers.weapon_ext.scounter == SSHOTGUN_ALTFIRE_COUNT)
			{
				//gi.bprintf(PRINT_HIGH, "scount = 3\n");
				if (ent->client->ps.gunframe == 12)
				{

					//ent->client->pers.secondary ^= SECONDARY_SUPERSHOTGUN;
					//ent->client->pers.scount = 0;
					ent->client->weaponstate = WEAPON_SWITCH_MODE;
					ent->client->ps.gunframe = 57;
				}
				ent->client->ps.gunframe++;
				return;
			}
		}
		else
		{
			if (ent->client->ps.gunframe == 7)
			{
				ent->client->ps.gunframe = 9;
			}
			else if (ent->client->ps.gunframe == 12)
			{
				ent->client->ps.gunframe++;
				return;
			}
			else
			{
				ent->client->ps.gunframe++;
				if (ent->client->ps.gunframe == 12)
					return;
			}
		}
		if (ent->client->ps.gunframe == 12)
			return;
	}
	vec3_t origin, angles;



	ent->client->kick_angles[0] += -4 * (ent->client->pers.weapon_ext.scounter + 1);
	VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	AngleVectors(angles, forward, right, NULL);

	VectorScale(forward, -4, origin);
	VectorAdd(origin, ent->client->kick_origin, ent->client->kick_origin);
	VectorSet(offset, 0, 6, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);


	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2)
			{
				damage *= 4;
				kick *= 4;
				ent->client->pers.quad_health -= damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				kick *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];

	v[YAW] = ent->client->v_angle[YAW] - 5;
	v[ROLL] = ent->client->v_angle[ROLL];

	v[PITCH] += ent->client->v_dmg_pitch;
	v[ROLL] += ent->client->v_dmg_roll;
	v[YAW] += ent->client->v_dmg_yaw;

	if (ent->client->pers.weapon_ext.scounter > 1)
		VectorMA(v, 0.5, ent->client->kick_angles, v);
	AngleVectors(v, forward, NULL, NULL);
	fire_shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);
	v[YAW] = v[YAW] + 10;
	AngleVectors(v, forward, NULL, NULL);
	fire_shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_SSHOTGUN | is_silenced);
	gi.multicast(start, MULTICAST_PVS);
	/*return;
	if (!(ent->client->pers.secondary & SECONDARY_SHOTGUN))
		ent->client->ps.gunframe += 2;
	else if ( ent->client->ps.gunframe == 7)
	{
		ent->client->ps.gunframe += 3;
	}*/
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun(edict_t *ent)
{
	static int	pause_frames[] = { 29, 42, 57, 0 };
	static int	fire_frames[] = { 7, 12, 0 };

	Weapon_Generic(ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire(edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	vec3_t angles;

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

	if (item_mod->value)
	{
		if (is_quad)
		{
			if (ent->client->pers.quad_health > damage * 0.2)
			{
				damage *= 4;
				kick *= 4;
				ent->client->pers.quad_health -= damage * 0.2;
			}
			else
			{
				damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				kick *= 4 - ((ent->client->pers.quad_health - (damage * 0.2)) / (damage * 0.2));
				ent->client->pers.quad_health = 0;
			}
		}
	}
	else
	{
		damage *= 4;
		kick *= 4;
	}

	VectorCopy(ent->client->v_angle, angles);
	angles[PITCH] += ent->client->v_dmg_pitch;
	angles[ROLL] += ent->client->v_dmg_roll;
	angles[YAW] += ent->client->v_dmg_yaw;
	AngleVectors(angles, forward, right, NULL);

	VectorScale(forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail(ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_RAILGUN | is_silenced);
	gi.multicast(start, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun(edict_t *ent)
{
	static int	pause_frames[] = { 56, 0 };
	static int	fire_frames[] = { 4, 0 };

	Weapon_Generic(ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire(edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t  angles;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;
	if (ent->client->ps.gunframe == 18)
	{
		if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_NORMAL)
		{
			ent->client->ps.gunframe++;
			return;
		}
		else
		{
			// cells can go down during windup (from power armor hits), so
			// check again and abort firing if we don't have enough now
			if (ent->client->pers.inventory[ent->client->ammo_index] < 10)
			{
				ent->client->ps.gunframe++;
				return;
			}

			if (item_mod->value)
			{
				if (is_quad)
				{
					if (ent->client->pers.quad_health > damage * 0.4)
					{
						damage *= 4;
						ent->client->pers.quad_health -= damage * 0.4;
					}
					else
					{
						damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.4)) / (damage * 0.4));
						ent->client->pers.quad_health = 0;
					}
				}
			}
			else
			{
				damage *= 4;
			}

			VectorCopy(ent->client->v_angle, angles);
			angles[PITCH] += ent->client->v_dmg_pitch;
			angles[ROLL] += ent->client->v_dmg_roll;
			angles[YAW] += ent->client->v_dmg_yaw;
			AngleVectors(angles, forward, right, NULL);

			VectorScale(forward, -2, ent->client->kick_origin);

			// make a big pitch kick with an inverse fall
			ent->client->v_dmg_pitch = -10;
			ent->client->v_dmg_roll = crandom() * 4;
			ent->client->v_dmg_time = level.time + 0.2;

			VectorSet(offset, 8, 8, ent->viewheight - 16);
			P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
			fire_bfg(ent, start, forward, damage / 4, 1500, 200);
			//gi.bprintf(PRINT_HIGH, "FAST BFG SHOT");
			ent->client->ps.gunframe++;

			PlayerNoise(ent, start, PNOISE_WEAPON);

			if (!((int)dmflags->value & DF_INFINITE_AMMO))
				ent->client->pers.inventory[ent->client->ammo_index] -= 10;
		}
	}
	if (ent->client->ps.gunframe == 9)
	{
		if (ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_FAST)
		{
			ent->client->ps.gunframe = 18;
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/Bfg__f2y.wav"), 1, ATTN_NORM, 0);
			return;
		}
		if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange(ent);
			ent->client->ps.gunframe = 55;
			ent->client->weaponstate = WEAPON_DROPPING;
			return;
		}
		else
		{


		}
		// send muzzle flash
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_BFG | is_silenced);
		gi.multicast(ent->s.origin, MULTICAST_PVS);
		//gi.bprintf(PRINT_HIGH, "BFG MUZZLEFLASH");
		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}
	if (ent->client->ps.gunframe == 17)
	{
		// cells can go down during windup (from power armor hits), so
		// check again and abort firing if we don't have enough now
		if (ent->client->pers.inventory[ent->client->ammo_index] < 50 && ent->client->pers.weapon_ext.mode == WEAPON_MODE_BFG_FAST)
		{
			ent->client->ps.gunframe++;
			return;
		}

		if (item_mod->value)
		{
			if (is_quad)
			{
				if (ent->client->pers.quad_health > damage * 0.4)
				{
					damage *= 4;
					ent->client->pers.quad_health -= damage * 0.4;
				}
				else
				{
					damage *= 4 - ((ent->client->pers.quad_health - (damage * 0.4)) / (damage * 0.4));
					ent->client->pers.quad_health = 0;
				}
			}
		}
		else
		{
			damage *= 4;
		}

		VectorCopy(ent->client->v_angle, angles);
		angles[PITCH] += ent->client->v_dmg_pitch;
		angles[ROLL] += ent->client->v_dmg_roll;
		angles[YAW] += ent->client->v_dmg_yaw;
		AngleVectors(angles, forward, right, NULL);

		VectorScale(forward, -2, ent->client->kick_origin);

		// make a big pitch kick with an inverse fall
		ent->client->v_dmg_pitch = -40;
		ent->client->v_dmg_roll = crandom() * 8;
		ent->client->v_dmg_time = level.time + DAMAGE_TIME;

		VectorSet(offset, 8, 8, ent->viewheight - 16);
		P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
		fire_bfg(ent, start, forward, damage, 400, damage_radius);
		//gi.bprintf(PRINT_HIGH, "NORMAL BFG SHOT");
		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);

		if (!((int)dmflags->value & DF_INFINITE_AMMO))
			ent->client->pers.inventory[ent->client->ammo_index] -= 50;
	}
}

void Weapon_BFG(edict_t *ent)
{
	static int	pause_frames[] = { 39, 45, 50, 55, 0 };
	static int	fire_frames[] = { 9, 17,18, 0 };

	Weapon_Generic(ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================
