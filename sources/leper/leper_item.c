#include "g_local.h"
#include "leper.h"

qboolean LeperPickUpArm (edict_t *ent, edict_t *other)
{
	
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	if(other->rightarm && other->leftarm)
	{
		ent = Drop_Item (other, FindItemByClassname ("item_arm"));
		return true;
	}
	else
	{
		if(!other->rightarm)
		{
			
			if(!other->leftarm && !other->rightarm)
			{
				other->rightarm=20;
				NoAmmoWeaponChange (other);
			}
			else
				other->rightarm = 20;

			return true;
		}
		
		if(!other->leftarm)
		{
			
			if(!other->leftarm && !other->rightarm)
			{
				other->leftarm=20;
				NoAmmoWeaponChange (other);
			}
			else
				other->leftarm = 20;

			return true;
		}
	return true;
	}
}
qboolean LeperPickUpLeg (edict_t *ent, edict_t *other)
{
	
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	if(other->rightleg && other->leftleg)
	{
		
		ent = Drop_Item (other, FindItemByClassname ("item_leg"));
		return true;
	}
	else
	{
		if(!other->rightleg)
		{	
			other->rightleg=40;
			Leper_NoLegs (other);
			return true;
		}
		
		if(!other->leftleg)
		{
			other->leftleg=40;
			Leper_NoLegs (other);
			return true;
		}
	}

	return false;
}
qboolean LeperPickUpGoldenLeg (edict_t *ent, edict_t *other)
{
	
	other->golden = true;
	owned = true;
	gi.bprintf (PRINT_HIGH, "%s has the golden leg!\n", other->client->pers.netname);
	ent->classname = "owned_goldenleg";
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	if(other->rightleg && other->leftleg)
	{
		ent = Drop_Item (other, FindItemByClassname ("item_leg"));
		return true;
	}
	else
	{
		if(!other->rightleg)
		{	
			other->rightleg=40;
			Leper_NoLegs (other);
			return true;
		}
		
		if(!other->leftleg)
		{
			other->leftleg=40;
			Leper_NoLegs (other);
			return true;
		}
	}

	return false;
}
qboolean LeperPickUpHead (edict_t *ent, edict_t *other)
{
	
	
	if(other->head)
	return false;
	else
	{
	other->head = 60;
	return true;
	}
}