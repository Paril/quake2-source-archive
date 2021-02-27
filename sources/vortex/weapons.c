#include "g_local.h"

//***********************************************************
//		Weapon Master class bonus
//***********************************************************
qboolean GiveWeaponMasterUpgrade(edict_t *ent, int WeaponIndex, int ModIndex)
{
	weapon_t *weapon;
	int maxLevel = 40;		//All hard maximums for the weapon master are set to this number.

	if (generalabmode->value == 0) // No weapon master bonuses in non-general ab mode.
		return false;

	//Point to the correct weapon
	weapon = &ent->myskills.weapons[WeaponIndex];

	//Don't crash
	if (ModIndex < 0 || ModIndex >= MAX_WEAPONMODS)
		return false;

	//Set the weapon master's bonus	
	switch(WeaponIndex)
	{
	case WEAPON_BLASTER:
		if(ModIndex < 3)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_SHOTGUN:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_SUPERSHOTGUN:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_MACHINEGUN:
		if(ModIndex < 3)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_CHAINGUN:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_GRENADELAUNCHER:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_ROCKETLAUNCHER:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_HYPERBLASTER:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_RAILGUN:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_BFG10K:
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_SWORD:
		if(ModIndex < 4 && ModIndex != 1)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_20MM:
		if(ModIndex < 3)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	case WEAPON_HANDGRENADE:	//(increased damage and radius)
		if(ModIndex == 0 || ModIndex == 2)
		{
			weapon->mods[ModIndex].soft_max = 20;
			weapon->mods[ModIndex].hard_max = maxLevel;
		}
		else return false;
		break;
	}
	return true;
}

//***********************************************************
//		Reset player's weapon maximums
//***********************************************************
void resetWeapons(edict_t *ent)
{
	int i, j;
    
	for (i = 0; i < MAX_WEAPONS; ++i)
	{
		for (j = 0; j < MAX_WEAPONMODS; ++j)
		{
			//Reset the current level (make it equal to the user's hard upgrade level)
			ent->myskills.weapons[i].mods[j].current_level = ent->myskills.weapons[i].mods[j].level;

			//Update the player's max levels ONLY IF they need it (ex: not loading a weapon from their player file)
			if(ent->myskills.weapons[i].mods[j].soft_max == 0 || ent->myskills.weapons[i].mods[j].hard_max == 0)
			{
				//Weapon masters get a bonus to some upgrades
				if (ent->myskills.class_num != CLASS_WEAPONMASTER || !GiveWeaponMasterUpgrade(ent, i, j))
				{
					//Sword gets an extra bonus
					if ((j < 3) || ((j == 3) && (i == WEAPON_SWORD)))
					{
							ent->myskills.weapons[i].mods[j].soft_max = 20;
							ent->myskills.weapons[i].mods[j].hard_max = 30;
					}
					else
					{
						ent->myskills.weapons[i].mods[j].soft_max = 1;
						ent->myskills.weapons[i].mods[j].hard_max = 1;
					}
				}
			}
		}
	}
}
//***********************************************************