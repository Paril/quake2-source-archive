/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_combat.c,v $
 *   $Revision: 1.53 $
 *   $Date: 2002/07/23 22:48:26 $
 *
 ***********************************

Copyright (C) 2002 Vipersoft

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

// g_combat.c

#include "g_local.h"
#include "g_cmds.h"
#include "allfuncs.h"

#define LEG_DAMAGE		(height/2) - abs(targ->mins[2]) - 3 
#define STOMACH_DAMAGE	(height/1.6) - abs(targ->mins[2]) 
#define CHEST_DAMAGE	(height/1.4) - abs(targ->mins[2]) 


#define WIDTH 0
#define DEPTH 1
#define HEIGHT 2

#define SPAWN_CAMP_TIME	10 //seconds
/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (edict_t *targ, edict_t *inflictor)
{
	vec3_t	dest;
	trace_t	trace;

	if (targ->client &&
		targ->client->resp.team_on &&
		targ->client->resp.mos == MEDIC &&
	    invuln_medic->value == 1)
	   return false;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
		trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}
	
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);

	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);

	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);

	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (inflictor->s.origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);

	if (trace.fraction == 1.0)
		return true;


	return false;
}


/*
============
Killed
============
*/
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;

	targ->enemy = attacker;

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
//		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type

		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))

		{
			level.killed_monsters++;

			if (coop->value && attacker->client)
				attacker->client->resp.score++;

			// medics won't heal monsters that they kill themselves

			if (strcmp(attacker->classname, "monster_medic") == 0)

				targ->owner = attacker;

		}
	}

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

/*	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		targ->touch = NULL;
		monster_death_use (targ);
	}*/

	// pbowens: quick hack for misplaced artys and grenades
	if (targ->client && attacker && !attacker->client && attacker->owner && attacker->owner->client)
		attacker = attacker->owner;
	if (targ->client && inflictor && inflictor != attacker && !inflictor->client && inflictor->owner && inflictor->owner->client)
		attacker = inflictor->owner;


	if(!targ->deadflag)
	{
		
	// pbowens: suicide gives other team kill
		if (attacker == targ || // themselves
			attacker == NULL || 
			attacker == world )// suicide/ unfotunate death
			team_list[(targ->client->resp.team_on->index) ? 0 : 1]->kills++;

		// pbowens: undo the previous kill count if changeteam
		if (targ->client && targ->client->resp.changeteam)
			team_list[(targ->client->resp.team_on->index) ? 0 : 1]->kills--;


		if((targ->client)					&&
			(attacker->client)				&&
			(targ->client->resp.team_on)	&& 
			(attacker->client->resp.team_on))
		{

			if (attacker != targ && attacker->client->resp.team_on == targ->client->resp.team_on) {
				if (team_kill->value == 2)
					attacker->client->penalty = PENALTY_TEAM_KILL;
			}
			
			if (attacker->client->resp.team_on != targ->client->resp.team_on)
				attacker->client->resp.team_on->kills++;
			
			targ->client->resp.team_on->losses++;

			if (spawn_camp_check->value)
			{
				if (!spawn_camp_time->value)
					 gi.cvar_set("spawn_camp_time", "2");

				if ((level.time - targ->client->spawntime) < spawn_camp_time->value) // how long it's been since they spawns
				{
					if (attacker->client->resp.killtime) {

						attacker->client->resp.oldkilltime = attacker->client->resp.killtime;
						attacker->client->resp.killtime = level.time;

						if ((attacker->client->resp.killtime - attacker->client->resp.oldkilltime) <= SPAWN_CAMP_TIME + ( (attacker->delay) ? attacker->delay : RI->value) )
						{
							if (targ->client->ps.pmove.pm_type != PM_DEAD)
								attacker->client->penalty= PENALTY_SPAWN_CAMP; // make sure to count respawn time
						}

					}
					else
						attacker->client->resp.killtime = level.time;
				}
			}
		}
	}

	if (targ->client)
	{
		//if (targ->client->resp.team_on  &&  targ->client->resp.mos)
		//	targ->client->resp.team_on->mos[targ->client->resp.mos]->available++;

		/*
		if (targ->client->resp.mos==OFFICER)
			targ->client->resp.bkupmos=targ->client->resp.team_on->officer_mos;
			*/

		//targ->client->resp.mos=targ->client->resp.bkupmos;
		//targ->client->limbo_mode=true;

		if (!targ->deadflag)
			targ->client->forcespawn = (level.framenum + FORCE_RESPAWN);
	}

	targ->die_time=0;
	targ->die (targ, inflictor, attacker, damage, point);
}

/*
================
SpawnDamage
================
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage)
{
	if (damage > 255)
		damage = 255;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (type);
//	gi.WriteByte (damage);
	gi.WritePosition (origin);
	gi.WriteDir (normal);
	gi.multicast (origin, MULTICAST_PVS);
}


/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack
point		point at which the damage is being inflicted
normal		normal vector from that point
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_ENERGY			damage is from an energy based weapon
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_BULLET			damage is from a bullet (used for ricochets)
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			power_armor_type;
	int			index;
	int			damagePerCell;
	int			pa_te_type;
	int			power;
	int			power_used;

	if (!damage)
		return 0;

	client = ent->client;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	if (client)
	{
		power_armor_type = PowerArmorType (ent);
		if (power_armor_type != POWER_ARMOR_NONE)
		{
			index = ITEM_INDEX(FindItem("Cells"));
			power = client->pers.inventory[index];
		}
	}
	else if (ent->svflags & SVF_MONSTER)
	{
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
	}
	else
		return 0;

	if (power_armor_type == POWER_ARMOR_NONE)
		return 0;
	if (!power)
		return 0;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		vec3_t		vec;
		float		dot;
		vec3_t		forward;

		// only works if damage point is in front
		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorSubtract (point, ent->s.origin, vec);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if (dot <= 0.3)
			return 0;

		damagePerCell = 1;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = damage / 3;
	}
	else
	{
		damagePerCell = 2;
		pa_te_type = TE_SHIELD_SPARKS;
		damage = (2 * damage) / 3;
	}

	save = power * damagePerCell;
	if (!save)
		return 0;
	if (save > damage)
		save = damage;

	SpawnDamage (pa_te_type, point, normal, save);
	ent->powerarmor_time = level.time + 0.2;

	power_used = save / damagePerCell;

	if (client)
		client->pers.inventory[index] -= power_used;
	else
		ent->monsterinfo.power_armor_power -= power_used;
	return save;
}

static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
	gclient_t	*client;
	int			save;
	int			index;
	gitem_t		*armor;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	index = ArmorIndex (ent);
	if (!index)
		return 0;

	armor = GetItemByIndex (index);

	if (dflags & DAMAGE_ENERGY)
		save = ceil(((gitem_armor_t *)armor->info)->energy_protection*damage);
	else
		save = ceil(((gitem_armor_t *)armor->info)->normal_protection*damage);
	if (save >= client->pers.inventory[index])
		save = client->pers.inventory[index];

	if (!save)
		return 0;

	client->pers.inventory[index] -= save;
	SpawnDamage (te_sparks, point, normal, save);

	return save;
}

void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{
	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
		return;

	if (attacker == targ || attacker == targ->enemy)
		return;

	// if we are a good guy monster and our attacker is a player
	// or another good guy, do not get mad at them
	if (targ->monsterinfo.aiflags & AI_GOOD_GUY)
	{
		if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
			return;
	}

	// we now know that we are not both good guys

	// if attacker is a client, get mad at them because he's good and we're not
	if (attacker->client)
	{
		// this can only happen in coop (both new and old enemies are clients)
		// only switch if can't see the current enemy
		if (targ->enemy && targ->enemy->client)
		{
			if (visible(targ, targ->enemy))
			{
				targ->oldenemy = attacker;
				return;
			}
			targ->oldenemy = targ->enemy;
		}
		targ->enemy = attacker;
		//if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		//	FoundTarget (targ);
		return;
	}

	// it's the same base (walk/swim/fly) type and a different classname and it's not a tank
	// (they spray too much), get mad at them
	if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM))) &&
		 (strcmp (targ->classname, attacker->classname) != 0) &&
		 (strcmp(attacker->classname, "monster_tank") != 0) &&
		 (strcmp(attacker->classname, "monster_supertank") != 0) &&
		 (strcmp(attacker->classname, "monster_makron") != 0) &&
		 (strcmp(attacker->classname, "monster_jorg") != 0) )
	{
		if (targ->enemy)
			if (targ->enemy->client)
				targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		//if (!(targ->monsterinfo.aiflags & AI_DUCKED))
		//	FoundTarget (targ);
	}
	else
	// otherwise get mad at whoever they are mad at (help our buddy)
	{
		if (targ->enemy)
			if (targ->enemy->client)
				targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

qboolean In_Vector_Range(vec3_t point, vec3_t origin, 
						 float x_range,
						 float y_range,
						 float z_range)
{
	vec3_t temp;

	VectorSubtract(point,origin, temp);

	if( (abs(temp[0])>x_range) ) return false;
	if( (abs(temp[1])>y_range) ) return false;
	if( (abs(temp[2])>z_range) ) return false;
	return true;
}

int Damage_Loc(edict_t *targ, vec3_t point)
{	float min_x = targ->s.origin[0] + targ->mins[0] - 0.1;
	float max_x = targ->s.origin[0] + targ->maxs[0] + 0.1;
	float min_y = targ->s.origin[1] + targ->mins[1] - 0.1;
	float max_y = targ->s.origin[1] + targ->maxs[1] + 0.1;
	float min_z = targ->s.origin[2] + targ->mins[2] - 0.1;
	float max_z = targ->s.origin[2] + targ->maxs[2] + 0.1;

	// rezmoth - impact debug info
	//gi.dprintf("mins  (%f, %f, %f)\n", targ->mins[0], targ->mins[1], targ->mins[2]);
	//gi.dprintf("maxs  (%f, %f, %f)\n", targ->maxs[0], targ->maxs[1], targ->maxs[2]);
	//gi.dprintf("point (%f, %f, %f)\n", point[0], point[1], point[2]);
	//gi.dprintf("Pmin  (%f, %f, %f)\n", min_x, min_y, min_z);
	//gi.dprintf("Pmax  (%f, %f, %f)\n", max_x, max_y, max_z);

	switch (targ->stanceflags)
	{
	case STANCE_STAND:
		if (point[2] > min_z + 46)
		{
			//gi.dprintf("head\n");
			return HEAD_WOUND;
		} else if (point[2] > min_z + 37 && point[2] < min_z + 46) {
			//gi.dprintf("chest\n");
			return CHEST_WOUND;
		} else if (point[2] > min_z + 28 && point[2] < min_z + 37) {
			// we would check for flamethrower here but the box is not rotated so the
			// player would have to be facing north for the code to work
			// for now we could just treat all fatal chest wounds as a flamethrower hit
			//gi.dprintf("stomache\n");
			return STOMACHE_WOUND;
		} else if (point[2] < min_z + 28) {
			//gi.dprintf("leg\n");
			return LEG_WOUND;
		}
		break;
	case STANCE_DUCK:
		if (point[2] > min_z + 34)
		{
			//gi.dprintf("head\n");
			return HEAD_WOUND;
		} else if (point[2] > min_z + 26 && point[2] < min_z + 34) {
			//gi.dprintf("chest\n");
			return CHEST_WOUND;
		} else if (point[2] > min_z + 16 && point[2] < min_z + 26) {
			//gi.dprintf("stomache\n");
			return STOMACHE_WOUND;
		} else if (point[2] < min_z + 18) {
			//gi.dprintf("leg\n");
			return LEG_WOUND;
		}
		break;
	case STANCE_CRAWL:
		// since crawl boxes are not rotated, you would have to face north for this to work
		// for this reason, we simply treat all prone hits as stomache wounds so the damage
		// is lessened because you are being sneaky and hard to hit
		return STOMACHE_WOUND;
		break;
	}

	// used to ensure that even leaks are counted for some damage
	// this prevents the hitboxes from not being hit even if traced
	//gi.dprintf("default wound\n");
	return STOMACHE_WOUND;

	/*
	float height, width, depth;//, temp;

	float leg_x,	leg_y,		leg_z;
	float stom_x,	stom_y,		stom_z;
	float head_x,	head_y,		head_z;
	float chest_x,	chest_y,	chest_z;
	float near_x,	near_y,		near_z; //Wheaty: determine when a bullet 'almost' hits.

	width  = abs(targ->mins[WIDTH])  + targ->maxs[WIDTH];		//0 x
	depth  = abs(targ->mins[DEPTH])  + targ->maxs[DEPTH];		//1 y
	height = abs(targ->mins[HEIGHT]) + targ->maxs[HEIGHT];		//2 z

	//gi.dprintf("h%f:w%f:d%f\n", height, width, depth);

	near_x = width;
	near_y = depth;
	near_z = height;


	switch (targ->stanceflags)
	{
	case STANCE_STAND:
	case STANCE_DUCK:
		leg_x = (width * 0.75) / 2;						// width
		leg_y = depth / 2;								// depth
		leg_z = targ->mins[HEIGHT]   + (height * 0.4);	// height

		stom_x = (width - 2) / 2;
		stom_y = depth / 2;
		stom_z = targ->mins[HEIGHT]  + (height * 0.4);

		chest_x = width / 2;
		chest_y = depth / 2;
		chest_z = targ->mins[HEIGHT] + (height * 0.75);

		head_x = (width *.5) / 2;
		head_y = depth / 2;
		head_z = targ->mins[HEIGHT]  + height;

		break;

	case STANCE_CRAWL:
		leg_x = (width * 0.75) / 2;
		leg_y = depth / 2;
		leg_z = targ->mins[HEIGHT]	 + (height * 0.4);

		stom_x = (width - 2) / 2;
		stom_y = depth / 2;
		stom_z = targ->mins[HEIGHT]	 + (height * 0.4);

		chest_x = width / 2;
		chest_y = depth / 2;
		chest_z = targ->mins[HEIGHT] + (height * 0.75);

		head_x = (width * 0.5) / 2;
		head_y = depth / 2;
		head_z = targ->mins[HEIGHT]  + height;

		break;
	}


	if (In_Vector_Range(point, targ->s.origin, leg_x, leg_y, leg_z) )
	{ //leg damage 
         
		//gi.dprintf("leg x%f:y%f:z%f\n", leg_x, leg_y, leg_z); 
		return LEG_WOUND;
	}

	if (In_Vector_Range(point, targ->s.origin, stom_x, stom_y, stom_z) )
	{ //stomach damage 

		//gi.dprintf("stom x%f:y%f:z%f\n", stom_x, stom_y, stom_z); 
		return STOMACHE_WOUND;
	}

	if (In_Vector_Range(point,targ->s.origin, chest_x, chest_y, chest_z) )
	{ //chest damage 

		//gi.dprintf("chest x%f:y%f:z%f\n", chest_x, chest_y, chest_z); 
		return CHEST_WOUND;
	}

	if (In_Vector_Range(point,targ->s.origin, head_x, head_y, head_z) )
	{ //head damage 

		//gi.dprintf("head x%f:y%f:z%f\n", head_x, head_y, head_z); 
		return HEAD_WOUND;
	}

	if (In_Vector_Range(point,targ->s.origin,near_x,near_y,near_z))
	{ //Wheaty: Near miss

		return NEAR_MISS;
	}
	else
		return 0;
	*/
}

#define HELMET_SAVE 85		//15% chance helmet will deflect the round
#define	BLEND_TIME 2		// How long the player is affected by damage.. (seconds)
//bcass start - 3% chance of being shot/function to do it
#define DROP_SHOT 97

//void Drop_Shot (edict_t *ent, gitem_t *item);

void Drop_Shot (edict_t *ent, gitem_t *item)
{
	int		index;

	if (!item)
		return;

	index = ITEM_INDEX(item);

	//pbowens: stop firing
	ent->client->buttons &= ~BUTTON_ATTACK;
	ent->client->latched_buttons &= ~BUTTON_ATTACK;
	ent->client->weaponstate = WEAPON_READY;

	Use_Weapon (ent, FindItem("Fists"));
	
	// wheaty: fix for drop-shot spam bug
	if (ent->client->pers.inventory[index])
	{
		Drop_Item (ent, item);
		ent->client->pers.inventory[index] = 0;

		gi.sound (ent, CHAN_BODY, gi.soundindex ("misc/drop.wav"), 1, ATTN_NORM, 0);
		gi.centerprintf(ent, "SHIT! YOU DROPPED YOUR WEAPON!!\n");
	}
}
//bcass end

void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take,

				save,
				asave,
//	int			psave;
				te_sparks,
				result,
				height,
				randnum,
				
				wound_location, 
				die_time;
    qboolean saved=false;
	
	if (IsValidPlayer(targ) && level.time < targ->client->spawntime + invuln_spawn->value) // pbowens: invulnerability
		return;

	//faf
	if (attacker && targ &&
		targ->client && attacker->client &&
		targ != attacker &&
		!OnSameTeam (targ, attacker) &&
		mod != MOD_EXPLOSIVE)
		targ->client->last_wound_inflictor = attacker;



	wound_location = die_time = 0;
	height = abs(targ->mins[2]) + targ->maxs[2];

   	if (targ->client &&((mod == MOD_PISTOL) || 
						(mod == MOD_SHOTGUN) || 
						(mod == MOD_RIFLE) || 
						(mod == MOD_LMG) || 
						(mod == MOD_HMG) || 
						(mod == MOD_SUBMG) || 
						(mod == MOD_SNIPER) ||
						(mod == MOD_KNIFE)) ) 
	{ 
			
			result = Damage_Loc(targ, point);
			targ->client->damage_knockback = 20;

	//Wheaty: Panzer Deflection (too lazy to make it modular)
	// pbowens: moved to be damage loc-independant
	if (result != LEG_WOUND &&
		targ->client &&
		targ->stanceflags != STANCE_STAND &&
		targ->client->aim &&
		targ->client->resp.team_on &&
//		targ->client->resp.team_on->index == 1 && 
		targ->client->pers.weapon && 
		strcmp(targ->client->pers.weapon->classname, "weapon_panzer") == 0 && 
		(rand() % 100) < 5.0)
	{
		gi.sound   (targ, CHAN_BODY, gi.soundindex ("world/ric2.wav"), 1, ATTN_NORM, 0);
		gi.cprintf (targ, PRINT_HIGH, "DEFLECTION\n");
		damage=0;
		result = 0; // make it nothing

		//Wheaty: Spark at point of deflection
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPARKS);
		gi.WritePosition (point);
		gi.WriteDir (point);
		gi.multicast (point, MULTICAST_PVS);

		targ->client->damage_div=0;

		targ->client->kick_angles[0] += 3;
		targ->client->kick_angles[1] += 3;
		targ->client->kick_angles[2] -= 3;



		return;
	}

	//Wheaty: Per Darwin's request... SMG/LMG can no longer inflict headshots
	if ((mod == MOD_LMG || mod == MOD_SUBMG) && result == HEAD_WOUND)
		result = CHEST_WOUND;
			
	switch(result)
	{		
		case LEG_WOUND:
			damage*=1.15;
			//if(targ->client) gi.cprintf(targ,PRINT_HIGH,"You've been hit in the leg!\n");
			wound_location |= LEG_WOUND;

			if (targ->client)
				targ->client->damage_div=1.7;

			gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hitleg.wav"), 1, ATTN_NORM, 0);
			
			break;

		case STOMACHE_WOUND:
			damage*=1.5;//0.75
			//if(targ->client) gi.cprintf(targ,PRINT_HIGH,"You've been hit in the stomach!\n");
			wound_location |=STOMACHE_WOUND;
			if(!targ->die_time)
				die_time=level.time+5;
			else
				die_time-=20;
			//targ->enemy=attacker;

//bcass start - random dropping weapon
			srand(rand());
			randnum=rand()%100;

				// wheaty: Don't let drop shot affect Morphine/Flamethrower/Fists/Binocs
				if(randnum > DROP_SHOT && IsValidPlayer(targ) && 
					targ->client->pers.weapon && targ->client->pers.weapon->classname &&
					(strcmp(targ->client->pers.weapon->classname, "weapon_fists") && 
					strcmp(targ->client->pers.weapon->classname, "weapon_Morphine") && 
					strcmp(targ->client->pers.weapon->classname, "weapon_flamethrower") &&
					strcmp(targ->client->pers.weapon->classname, "weapon_binoculars")))
				{
					Drop_Shot (targ, targ->client->pers.weapon);
				}
//bcass end
			
			if (targ->client)
				targ->client->damage_div=1.4;

			gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hittorso.wav"), 1, ATTN_NORM, 0);
			break;

		case CHEST_WOUND:

			//	else
				{

					damage*=2;//1.1;
					//if(targ->client) gi.cprintf(targ,PRINT_HIGH,"You've been hit in the chest!\n");
					wound_location |=CHEST_WOUND;

					if(!targ->die_time)
						die_time=level.time+5;
					else
						die_time-=45;

					
					if (targ->client)
					{
						targ->client->damage_div=1.3;
					}
				
					gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hittorso.wav"), 1, ATTN_NORM, 0);

				}

			break;

		case HEAD_WOUND:
			if(targ->client)
			{
				if(targ->client->pers.inventory[ITEM_INDEX(FindItem("Helmet"))]) //if they got helmet
				{
					srand(rand());
					randnum=rand()%100;
					if(randnum > HELMET_SAVE)
					{
						gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hithelm.wav"), 1, ATTN_NORM, 0);
						gi.cprintf(targ,PRINT_HIGH,"You lucky bastard! Your helmet deflected the shot!\n");
						damage = 0;
						targ->client->kick_angles[0] += 3;
						targ->client->kick_angles[1] -= 3;
						targ->client->kick_angles[2] += 3;
						targ->client->damage_div=0;
						saved=true;
					}
				}

				if(!saved)
				{
					if (!targ->deadflag) {
						gi.sound (targ, CHAN_BODY, gi.soundindex ("misc/hithead.wav"), 1, ATTN_NORM, 0);
						gi.cprintf(targ,PRINT_HIGH,"Your head's been shot off!\n");
					}
					damage *= 100;
				
				//Wheaty: This was missing, for some reason :p
					wound_location |= HEAD_WOUND;

				// Dont fade if death from headshot
					targ->client->resp.deathblend = 1;
				}
				else
				{
					//Wheaty: Spark at point of deflection
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPARKS);
					gi.WritePosition (point);
					gi.WriteDir (point);
					gi.multicast (point, MULTICAST_PVS);
				}


			}
					

			break;

		default: return;
		}	
	} 
	
 	if (!targ->takedamage)
		return;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if (attacker != NULL    &&		
		targ != attacker	&& 
		targ->client		&&
		attacker->client)
	{
		if (targ->client->resp.mos == MEDIC && invuln_medic->value == 1)
		{
			wound_location	= 0;
			die_time		= 0;
			damage			= 0;
		}

		if (OnSameTeam (targ, attacker))
		{
			if (team_kill->value == 1 || team_kill->value == 2) { // ff w/ penalties

				mod |= MOD_FRIENDLY_FIRE;
				targ->wound_location |= wound_location;
				targ->die_time       += die_time;

				//Wheaty: Spray Blood
				if (result == HEAD_WOUND)
				{
					if (!saved)
						SprayBlood(targ, point, dir, damage, mod);
				}
				else
				{
					SprayBlood(targ, point, dir, damage, mod);
				}

			} 
			else 
			{
				damage = 0;
				knockback = 0;
			}
		} else {

			targ->wound_location |= wound_location;
			targ->die_time       -= die_time;

			//Wheaty: Spray Blood
			if (result == HEAD_WOUND)
			{
				if (!saved)
					SprayBlood(targ, point, dir, damage, mod);
			}
			else
			{
				SprayBlood(targ, point, dir, damage, mod);
			}
		}

		WeighPlayer(targ);
	}

	meansOfDeath = mod;
	
	// easy mode takes half damage
	if (skill->value == 0 && deathmatch->value == 0 && targ->client)
	{
		damage *= 0.5;
		if (!damage)
			damage = 1;
	}

	client = targ->client;

	if (dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	VectorNormalize(dir);

// bonus damage for suprising a monster
	if (!(dflags & DAMAGE_RADIUS) && (targ->svflags & SVF_MONSTER) && (attacker->client) && (!targ->enemy) && (targ->health > 0))
		damage *= 2;

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP))
		{
			vec3_t	kvel;
			float	mass;

			if (targ->mass < 50)
				mass = 50;
			else
				mass = targ->mass;

		if (targ->client  && attacker == targ)
				VectorScale (dir, 1600.0 * (float)knockback / mass, kvel);	// the rocket jump hack...
			else
				VectorScale (dir, 500.0 * (float)knockback / mass, kvel);

			VectorAdd (targ->velocity, kvel, targ->velocity);
		}
	}

	take = damage;
	save = 0;

	//psave = CheckPowerArmor (targ, point, normal, take, dflags);
	//take -= psave;

	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	take -= asave;

	//treat cheat/powerup savings the same as armor
	asave += save;

	// team damage avoidance
	//if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
	//	return;

// do the damage
	if (take)
	{
		if ((targ->svflags & SVF_MONSTER) || (client))
			SpawnDamage (TE_BLOOD, point, normal, take);
		else
			SpawnDamage (te_sparks, point, normal, take);

		if (attacker) 
			targ->enemy=attacker;
		else if (mod==MOD_WOUND) 
			attacker=targ->enemy;
		
		targ->health = targ->health - take;
		
		if (targ->health <= 0)
		{
			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
			//if(mod==MOD_WOUND) targ->enemy=attacker;

			//gi.dprintf("attacker on i%\n",attacker->team_on->kills+5);

			Killed (targ, inflictor, attacker, take, point);
			return;
		}
	}

	if (targ->svflags & SVF_MONSTER)
	{
		M_ReactToDamage (targ, attacker);

		if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take))
		{
			targ->pain (targ, attacker, knockback, take);

			// nightmare mode monsters don't go into pain frames often
			if (skill->value == 3)
				targ->pain_debounce_time = level.time + 5;
		}

	}
	else if (client)
	{
		if (!(targ->flags & FL_GODMODE) && (take))
			targ->pain (targ, attacker, knockback, take);
	}
	else if (take)
	{
		if (targ->pain)
			targ->pain (targ, attacker, knockback, take);
	}


	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client)
	{

		client->damage_blendtime = (level.time + BLEND_TIME);
//		client->damage_parmor += psave;
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
	}

}


/*
============
T_RadiusDamage
============
*/
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL)
	{
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		points = damage - 0.5 * VectorLength (v);
		if (ent == attacker)
			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
			}
		}
	}

	//pbowens: for explosion effects, yes this assumes its an explosion
	SetExplosionEffect(inflictor, damage, radius);

	return;
}

void BloodSprayThink (edict_t *self)
{
	//Wheaty: Borrowed from AQ        
	G_FreeEdict(self);
}

void blood_spray_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//Wheaty: Borrowed from AQ

        if (other == ent->owner)
                return;
        ent->think = G_FreeEdict;
        ent->nextthink = level.time + 0.1;
}

void SprayBlood(edict_t *self, vec3_t point, vec3_t angle, int damage, int mod)
{

	//Wheaty: Borrowed from AQ
	
	edict_t *blood;
	int		speed;

	switch (mod)
	{
	
	case MOD_PISTOL:
		speed = 250;
		break;
	case MOD_SHOTGUN:
		speed = 300;
		break;
	case MOD_RIFLE:
		speed = 400;
		break;
	case MOD_LMG:
		speed = 400;
		break;
	case MOD_HMG:
		speed = 700;
		break;
	case MOD_SUBMG:
		speed = 500;
		break;
	case MOD_SNIPER:
		speed = 800;
		break;
	case 69:
		speed = 2000;
	default:
		speed = 500;
	}

	//Wheaty: To prevent fireworks bloodspray
	if (mod == MOD_PISTOL || mod == MOD_SHOTGUN || mod == MOD_RIFLE || mod == MOD_LMG
		|| mod == MOD_HMG || mod == MOD_SUBMG || mod == MOD_SNIPER || mod == MOD_KNIFE || mod == 69)
	{

	blood = G_Spawn();
    VectorNormalize(angle);
    VectorCopy (point, blood->s.origin);
    VectorCopy (angle, blood->movedir);

	vectoangles (angle, blood->s.angles);
    VectorScale (angle, speed, blood->velocity);

	//debug
	//gi.cprintf(self, PRINT_HIGH,"MOD: %d   VEL: %f\n", mod, VectorLength (blood->velocity));

    blood->movetype = MOVETYPE_TOSS;
    blood->clipmask = MASK_SHOT;
    blood->solid = SOLID_BBOX;
    blood->s.effects |= EF_GIB; 
    VectorClear (blood->mins);
    VectorClear (blood->maxs);
    blood->s.modelindex = gi.modelindex ("sprites/null.sp2");
    blood->owner = self;
    blood->nextthink = level.time + speed/200;
    blood->touch = blood_spray_touch;
    blood->think = BloodSprayThink;
    blood->dmg = damage; 
    blood->classname = "blood_spray";
        
    gi.linkentity (blood);

	}
}

// pbowens: since we cant actually tell when an explosion happens, call this after the WriteByte
void SetExplosionEffect (edict_t *inflictor, float damage, float radius) 
{
//	int		j;
	float	effect_radius;
	vec3_t	v;
	edict_t	*dmgef_ent = NULL;

	// pbowens: explosive effects 
	//	 NOTE: this assumes whatever calls T_RadiusDamage is an explosion
	effect_radius = radius + damage;

	//gi.dprintf("effect_radius: %f\n       radius: %f\n       damage: %f\n",
	//	effect_radius, radius, damage);

	while ((dmgef_ent = findradius(dmgef_ent, inflictor->s.origin, effect_radius)) != NULL)
	{
		if (!dmgef_ent->client)
			continue;

		dmgef_ent->client->dmgef_startframe = level.framenum;
		dmgef_ent->client->dmgef_sway_value = 999; // start the sway count
		
		VectorAdd (dmgef_ent->mins, dmgef_ent->maxs, v);
		VectorMA (dmgef_ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		dmgef_ent->client->dmgef_intensity = effect_radius - VectorLength (v);

		//gi.dprintf("VectorLength: %f (flash is < 275)\n", VectorLength (v));
		if (VectorLength (v) < 275)
			dmgef_ent->client->dmgef_flash = true;
		else
			dmgef_ent->client->dmgef_flash = false;


	//	gi.dprintf("dmgef_startframe = %i\n", dmgef_ent->client->dmgef_startframe);
	//	gi.dprintf("dmgef_intensity  = %f\n", dmgef_ent->client->dmgef_intensity);
	}
}
