#include "g_local.h"
#include "m_player.h"

void MatrixReplaceItems (edict_t *ent)
{
	if ((weaponban->value) && (ent->item->flags & IT_WEAPON))
		G_FreeEdict(ent);

	if (!strcmp(ent->classname, "item_quad"))
		ent->classname = (FindItem("Damage Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_invulnerability"))
		ent->classname = (FindItem("Speed Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_adrenaline"))
		ent->classname = (FindItem("Stamina Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_health_mega"))
		ent->classname = (FindItem("Health Upgrade"))->classname;

	if (!strcmp(ent->classname, "item_power_shield"))
		ent->classname = (FindItem("Damage Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_power_screen"))
		ent->classname = (FindItem("Damage Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_silencer"))
		ent->classname = (FindItem("Stamina Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_breather"))
		ent->classname = (FindItem("Stamina Upgrade"))->classname;
	if (!strcmp(ent->classname, "item_enviro"))
		ent->classname = (FindItem("Damage Upgrade"))->classname;
	
	if (!strcmp(ent->classname, "item_armor_shard"))
		ent->classname = (FindItem("Bullets"))->classname;
	if (!strcmp(ent->classname, "ammo_grenades"))
		ent->classname = (FindItem("Bullets"))->classname;
	if (!strcmp(ent->classname, "ammo_cells"))
		ent->classname = (FindItem("Medium Stamina"))->classname;
	if (!strcmp(ent->classname, "ammo_shells"))
		ent->classname = (FindItem("Small Stamina"))->classname;
	if (!strcmp(ent->classname, "ammo_slugs"))
		ent->classname = (FindItem("Large Stamina"))->classname;
	if (!strcmp(ent->classname, "ammo_rockets"))
		ent->classname = (FindItem("Medium Stamina"))->classname;

	if (!strcmp(ent->classname, "item_pack"))
		ent->classname = (FindItem("Bandolier"))->classname;
	if (!strcmp(ent->classname, "weapon_bfg"))
		ent->classname = (FindItem("Desert Eagle Pistol"))->classname;
	
	if (!strcmp(ent->classname, "weapon_grenadelauncher"))
		ent->classname =  (FindItem("Grenades"))->classname;
	if (!strcmp(ent->classname, "weapon_chaingun"))
		ent->classname =  (FindItem("Semi Mobile Chaingun"))->classname;
	if (!strcmp(ent->classname, "weapon_hyperblaster"))
		ent->classname =  (FindItem("m4 assault rifle"))->classname;
	if (!strcmp(ent->classname, "weapon_railgun"))
		ent->classname =  (FindItem("Sniper Rifle"))->classname;
	if (!strcmp(ent->classname, "weapon_machinegun"))
		ent->classname = (FindItem("mp5 machine gun"))->classname;
	if (!strcmp(ent->classname, "weapon_rocketlauncher"))
		ent->classname = (FindItem("Surface to Surface Missile rack"))->classname;
	if (!strcmp(ent->classname, "weapon_shotgun"))
		ent->classname = (FindItem("desert eagle pistol"))->classname;
	if (!strcmp(ent->classname, "weapon_supershotgun"))
		ent->classname = (FindItem("Pump Action Shotgun"))->classname;
	
}
void MatrixSetupItems (edict_t *ent)
{
/*	if (strcmp(ent->classname, "item_flag_team1") == 0 ||
		strcmp(ent->classname, "item_flag_team2") == 0)
	{
		ent->think = CTFFlagSetup;
		ent->nextthink = level.time + FRAMETIME*2;
	}*/
	if (strcmp(ent->classname, "item_matrix_damageup") == 0)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_BLUE);
		ent->s.effects |= EF_BLUEHYPERBLASTER;
	}
	if (strcmp(ent->classname, "item_matrix_healthup") == 0)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_GREEN);
		ent->s.effects |= EF_BFG;
	}
	if (strcmp(ent->classname, "item_matrix_staminaup") == 0)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED);
		ent->s.effects |= EF_PENT;
	}
	if (strcmp(ent->classname, "item_matrix_speedup") == 0)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN);
		ent->s.effects |= EF_HYPERBLASTER;
	}
}

qboolean Pickup_MatrixDamageUp (edict_t *ent, edict_t *other)
{
	if (other->healthlevel + other->staminalevel + other->damagelevel >= sv_maxlevel->value)
	{

		return false;
	}
	other->damagelevel += 1;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_MatrixSpeedUp (edict_t *ent, edict_t *other)
{

	if(strcmp(other->client->pers.weapon->classname, "weapon_knives") == 0 || 
		strcmp(other->client->pers.weapon->classname, "weapon_fists") == 0)
	{
	other->client->speed_framenum = level.framenum + 150;	

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
	}
	else
	return false;
}

qboolean Pickup_MatrixStaminaUp (edict_t *ent, edict_t *other)
{
	if (other->healthlevel + other->staminalevel + other->damagelevel >= sv_maxlevel->value)
	{

		return false;
	}
	other->staminalevel += 1;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_MatrixHealthUp (edict_t *ent, edict_t *other)
{
	if (other->healthlevel + other->staminalevel + other->damagelevel >= sv_maxlevel->value)
	{

		return false;
	}
	other->healthlevel += 1;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_MatrixStamina (edict_t *ent, edict_t *other)//this is FIXED!
{
	if (other->stamina < other->max_stamina)
	{
		other->stamina += ent->item->quantity;

		if (other->stamina > other->max_stamina)
			other->stamina = other->max_stamina;

		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);

		return true;
	}	
	else
		return false;
}
qboolean Pickup_MatrixPack (edict_t *ent, edict_t *other)
{
	int n;
	gitem_t *it;

	for(n=0; n<game.num_items;n++)
	{
		it = &itemlist[n];
		if ((it->flags & IT_WEAPON) && (it->ammo))
		{
			if (Q_stricmp(it->classname, "weapon_bfg")==0)
				continue;
				
			other->client->pers.inventory[ITEM_INDEX(it)]++;
			// Change me
			Add_Ammo(other, FindItem(it->ammo),  FindItem(it->ammo)->quantity);
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

void AkimboChangeWeapon (edict_t *ent)
{
	int i;
// SHOULD BE USED IN DEACTIVATING BOTH WEAPONS
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

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if(ent->client->pers.weapon->akimbo_model)
	ent->client->akimbo = true;
	
	if(ent->client->akimbo)
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->akimbo_model);
	else
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);


        if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}


        //SKULL
        /* Don't display weapon if chasetoggle is on */
        if (ent->client->chasetoggle)
		ent->client->ps.gunindex = 0;
        //END

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
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Matrix_Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it

	//if ((item == ent->client->pers.weapon)&&
	//	(item->akimbo_model)&&
	//	(ent->client->ps.gunindex == gi.modelindex(ent->client->pers.weapon->view_model)))
		// And its an akimbo gun And you have at least 2 guns
	//{
	//	ent->client->akimbo = true;
	//	ent->client->weaponstate = WEAPON_ACTIVATING;
	//	ent->client->ps.gunframe = 0;
	//	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->akimbo_model);
	//	return;
	//}
	
	// see if we're already using it


	
	// this bt switches between akimbo and single modes.
	if (item == ent->client->pers.weapon)
	if (item->akimbo_model)
	if(ent->client->akimbo)
	{
		ent->client->akimbo = false;
		ent->client->weaponstate = WEAPON_ACTIVATING;
		ent->client->ps.gunframe = 0;
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		return;
	}
	else
	{
		ent->client->akimbo = true;
		ent->client->weaponstate = WEAPON_ACTIVATING;
		ent->client->ps.gunframe = 0;
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->akimbo_model);
		return;
	}
	

	//this works.
	
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
	
	if (item->akimbo_model)
	{
		ent->client->akimbo = true;
	//	ent->client->weaponstate = WEAPON_ACTIVATING;
	//	ent->client->ps.gunframe = 0;
	//	ent->client->ps.gunindex = gi.modelindex(item->akimbo_model);
	}

	// change to this weapon when done
	ent->client->newweapon = item;

}