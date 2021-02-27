#include "g_local.h"

//FIXME: this code is a mess, clean me!
void cloak(edict_t *ent)
{
	if (!G_EntExists(ent))
		return; // if they aren't in the game or are a boss, abort

	if (!ent->client->cloakable || ent->myskills.abilities[CLOAK].current_level < 1 
		|| ent->myskills.abilities[CLOAK].disable)
	{
		if (ent->client->cloaking)
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->client->cloaking = false;
			ent->client->cloakable = false;
			if (ent->myskills.abilities[CLOAK].current_level < 1)
				gi.cprintf(ent, PRINT_HIGH, "You can't use cloaking, you never trained in it!\n");
		}
		return;
	}

	if (ent->svflags & SVF_NOCLIENT)
	{
		 if ((ent->client->pers.inventory[power_cube_index] >= CLOAK_DRAIN_AMMO) 
			 && (level.time > ent->lastdmg+1)) // stay cloaked as long as player doesn't deal damage
		 {
			 gi.dprintf("%.1f %.1f\n", level.time, ent->lastdmg);

				 ent->client->cloakdrain ++;
				 if (ent->client->cloakdrain == CLOAK_DRAIN_TIME)
				 {
						 ent->client->pers.inventory[power_cube_index] -= CLOAK_DRAIN_AMMO;
						 ent->client->cloakdrain = 0;
				 }
		 }
		 else
		 {
				 ent->svflags &= ~SVF_NOCLIENT;
				 ent->client->cloaking = false;
				 ent->client->cloakable = false;
		 }
	}
	else
	{
		 if (ent->client->cloaking && (ent->client->pers.inventory[power_cube_index] >= CLOAK_DRAIN_AMMO))
		 {
				 if (level.time > ent->client->cloaktime 
					 && (ent->client->pers.inventory[power_cube_index] >= CLOAK_COST))
				 {
						 ent->svflags |= SVF_NOCLIENT;
						 ent->client->cloakdrain = 0;
						 ent->client->pers.inventory[power_cube_index] -= CLOAK_COST;
						 gi.bprintf (PRINT_HIGH, "%s cloaked.\n", ent->client->pers.netname);
				 }
		 }
		 else if (ent->client->pers.inventory[power_cube_index] >= CLOAK_DRAIN_AMMO)
		 {
				 ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
				 ent->client->cloaking = true;
		 }
		 else
		 {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->client->cloaking = false;
			ent->client->cloakable = false;
		 }
	}

}