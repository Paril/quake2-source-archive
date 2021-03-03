/*
 *
 * UrbanCTF Locational Damage Routines
 *
 * Create on September, 27th
 * by Spk
 *
 * Copyright(c) Urban Software
 *
 */

#include "g_local.h"
#include "mod.h"

/*
 * Most of the main functions are called in g_combat.c, in the T_Damage routine
 */

/*
 * Defines
 */
#define LOC_NONE	0
#define LOC_CHEST	1
#define LOC_HEAD	2
#define LOC_LEGS	3
#define LOC_STOMAC	4

/*
 *
 * A leg shoot is HALF damage
 * A stomac shoot is 2/3 damage
 * A chest shoot is NORMAL damage
 * An head shoot is INSTANT DEATH
 *
 */

/*
 *
 * GetHitLocation
 *
 * Return (int) the type of location
 * related on where the shoot
 * was placed
 *
 * Point : impact of where the shoot hit
 * Ent : the Target of the shoot
 * Mod : what type of gun was used
 *
 */
int GetHitLocation (vec3_t point, edict_t *ent, int mod)
{
	vec3_t hLoc;

	if (!ent->client)
		return LOC_NONE;	//apply on players only

	//check for the weapon type the shoot was firied with
	if ((mod == MOD_BLASTER)		|| 
		(mod == MOD_SHOTGUN)		|| 
		(mod == MOD_SSHOTGUN)		|| 
		(mod == MOD_MACHINEGUN)		|| 
		(mod == MOD_CHAINGUN)		|| 
		(mod == MOD_HYPERBLASTER)	||
		(mod == MOD_RAILGUN))
	{

		VectorSubtract(point, ent->s.origin, hLoc);
		
		if (ent->maxs[2] <= 4) 
		{
			// target is crouching
			if (hLoc[2] > 0)
				return LOC_HEAD;
			else if (hLoc[2] < -6)
				return LOC_LEGS;
		}
		else 
		{
			// target is standing
			if (hLoc[2] > 16)
				return LOC_HEAD;
			else if (hLoc[2] < 4)
				return LOC_LEGS;
		}
		return LOC_CHEST;
	}
}

/*
 * 
 * ApplyLocationalSystem
 *
 * return the amount of damage
 * related on the impact origin of
 * the shoot
 *
 */
int ApplyLocationalSystem (edict_t *attacker, edict_t *targ, vec3_t point, int mod, int d_damage)
{
	int loc;
	int damage = d_damage;
	int r = random()*3;

	if (!targ->client)
		return d_damage;

	loc = GetHitLocation(point, targ, mod);

	if (loc == LOC_NONE)
		return d_damage;

	switch (loc)
	{
	case LOC_HEAD:
		if (attacker->client)
		{
			if (attacker->misc_time[MSG_TIME] < level.time)
			{
				if (r <= 10)
					gi.cprintf (attacker, PRINT_HIGH, "HEAD shot\n");
				else if (r <= 20)
					gi.cprintf (attacker, PRINT_HIGH, "You shot %s in the HEAD\n", targ->client->pers.netname);
				else
					gi.cprintf (attacker, PRINT_HIGH, "You've blown %s's head !\n", targ->client->pers.netname);

				attacker->misc_time[MSG_TIME] = level.time + 1;
			}
		}
		damage *= 6;
		break;
	case LOC_LEGS:
		if (attacker->client)
		{
			if (attacker->misc_time[MSG_TIME] < level.time)
			{
				if (r <= 10)
					gi.cprintf (attacker, PRINT_HIGH, "LEG shot\n");
				else if (r <= 20)
					gi.cprintf (attacker, PRINT_HIGH, "You shot %s in the Legs\n", targ->client->pers.netname);
				else
					gi.cprintf (attacker, PRINT_HIGH, "You've injured %s's legs !\n", targ->client->pers.netname);

				attacker->misc_time[MSG_TIME] = level.time + 1;
			}
		}
		damage *= 0.5;
		break;
	case LOC_CHEST:
		if (attacker->client)
		{
			if (attacker->misc_time[MSG_TIME] < level.time)
			{
				if (r <= 10)
					gi.cprintf (attacker, PRINT_HIGH, "CHEST shot\n");
				else if (r <= 20)
					gi.cprintf (attacker, PRINT_HIGH, "You shot %s in the chest !\n", targ->client->pers.netname);
				else
					gi.cprintf (attacker, PRINT_HIGH, "Nice shot in the chest !\n", targ->client->pers.netname);

				attacker->misc_time[MSG_TIME] = level.time + 1;
			}
		}
	default:
		return d_damage;
		break;
	}

	return damage;

}