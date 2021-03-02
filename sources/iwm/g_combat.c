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
// g_combat.c

#include "g_local.h"

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
void PumpBodies (edict_t *self)
{
	// The second he is "dead", we remove him.
	if (self->svflags & SVF_DEADMONSTER)
	{
		G_MuzzleFlash (self-g_edicts, self->s.origin,11); // do sparkle effect when body is removed from game
		G_FreeEdict (self);
		return;
	}

	self->nextthink2 = level.time + .1;
}
void MonsterObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);

void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;
	
/*	if (deathmatch->value)
		// Was this the XTransformer which just got shot down?
		if (Q_stricmp(targ->classname, "XTransformer")==0)
			if (G_EntExists(attacker) && (attacker!=targ->transformer)) 
			{
				safe_centerprintf(attacker,"7 Frags for killing a Gladiator!\n");
				attacker->client->resp.score += 7; 
			}*/

	targ->enemy = attacker;

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
//		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY))
		{
			level.killed_monsters++;
			if (coop->value && attacker->client)
				Frag_Get (attacker, 1);
			// medics won't heal monsters that they kill themselves
			if (attacker && strcmp(attacker->classname, "monster_medic") == 0)
				targ->owner = attacker;
		}
	}

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		// New feature: "pump" bodies out after a few seconds.
		targ->think2 = PumpBodies;
		targ->nextthink2 = level.time + .1;
		targ->touch = NULL;
		monster_death_use (targ);
		MonsterObituary (targ, inflictor, attacker);
	}

	if (targ->die)
		targ->die (targ, inflictor, attacker, damage, point);
}


/*
================
SpawnDamage
================
*/
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage, int flags)
{
	int i;

	if (damage > 255)
		damage = 255;

	if (damage > 100 && !flags & FL_GODMODE)
	{
		for (i = 0; i < 5; i++)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_MOREBLOOD);
		//	gi.WriteByte (damage);
			gi.WritePosition (origin);
			gi.WriteDir (normal);
			gi.multicast (origin, MULTICAST_PVS);
		}
	}
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
	int			index = 0;
	int			damagePerCell;
	int			pa_te_type;
	int			power = 0;
	int			power_used;

	if (!damage)
		return 0;

	client = ent->client;

	if (dflags & (DAMAGE_NO_ARMOR | DAMAGE_NO_POWER_ARMOR))		// PGM
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

	// etf rifle
	if (dflags & DAMAGE_NO_REG_ARMOR)
		save = (power * damagePerCell) / 2;
	else
		save = power * damagePerCell;

	if (!save)
		return 0;
	if (save > damage)
		save = damage;

	SpawnDamage (pa_te_type, point, normal, save, ent->flags);
	ent->powerarmor_time = level.time + 0.2;

	if (dflags & DAMAGE_NO_REG_ARMOR)
		power_used = (save / damagePerCell) * 2;
	else
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

	// ROGUE - added DAMAGE_NO_REG_ARMOR for atf rifle
	if (dflags & (DAMAGE_NO_ARMOR | DAMAGE_NO_REG_ARMOR))
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
	SpawnDamage (te_sparks, point, normal, save, ent->flags);

	return save;
}

void M_ReactToDamage (edict_t *targ, edict_t *attacker)
{
	if (!attacker || !(attacker->client) && !(attacker->svflags & SVF_MONSTER))
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
		targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

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
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
		return;
	}

	// it's the same base (walk/swim/fly) type and a different classname and it's not a tank
	// (they spray too much), get mad at them
	/*if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM))) &&
		 (strcmp (targ->classname, attacker->classname) != 0) &&
		 (strcmp(attacker->classname, "monster_tank") != 0) &&
		 (strcmp(attacker->classname, "monster_supertank") != 0) &&
		 (strcmp(attacker->classname, "monster_makron") != 0) &&
		 (strcmp(attacker->classname, "monster_jorg") != 0) )
	{*/
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	//}
	// if they *meant* to shoot us, then shoot back
	if (attacker->enemy == targ)
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
	// otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
	/*else if (attacker->enemy && attacker->enemy != targ)
	{
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}*/
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker)
{
//ZOID
	if (ctf->value && targ->client && attacker->client)
		if (targ->client->resp.ctf_team == attacker->client->resp.ctf_team &&
			targ != attacker)
			return true;
//ZOID

		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

qboolean IsProjectile (edict_t *i)
{
	if (Q_stricmp(i->classname, "firefly") == 0)
		return false;
	if (Q_stricmp(i->classname, "noclass") == 0)
		return false;
	if (Q_stricmp(i->classname, "hook") == 0)
		return false;

	if (i->movetype == MOVETYPE_FLYMISSILE)
		return true;
	else
		return false;
}

void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	int			take;
	int			save;
	int			asave;
	int			psave;
	int			te_sparks;

	if (!attacker)
		return;

	if (Q_stricmp(attacker->classname, "monster_car") == 0)
		attacker = attacker->activator;

	if (targ->client && targ->client->star_framenum > level.framenum)
		return;

	if (!targ->takedamage)
		return;

	if (iwm_editor->value && targ->client)
		return;

	/*if (targ->client && targ->client->deflector_framenum > level.framenum && IsProjectile(inflictor))
	{
		if (infront(targ, inflictor))
		{
			G_FreeEdict (inflictor);
			return;
		}
	}*/

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if ((targ != attacker))
	{
		if (coop->value)
			damage = 0;
		if (OnSameTeam (targ, attacker))
		{
			if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
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

//ZOID
//strength tech
	damage = CTFApplyStrength(attacker, damage);
//ZOID

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_BOUNCESTAY)  && (targ->movetype != MOVETYPE_RUBBER) && (targ->movetype != MOVETYPE_BOUNCENOSTOP) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP))
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
	
	if (Q_stricmp(inflictor->classname, "gravity_grenade") == 0 && targ->client)
	{
		targ->gravity_lift = level.time + 10.0;
		if (targ->client && attacker->client)
			safe_cprintf (targ, PRINT_HIGH, "Your gravitonation field was modified by %s!\n", attacker->client->pers.netname);
	}

	take = damage;
	save = 0;

	// check for godmode
	if ( (targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) )
	{
		take = 0;
		save = damage;
		SpawnDamage (te_sparks, point, normal, save, targ->flags);
	}

	// check for invincibility
	if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, SoundIndex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}
		take = 0;
		save = damage;
	}

//ZOID
//team armor protect
	if (ctf->value && targ->client && attacker->client &&
		targ->client->resp.ctf_team == attacker->client->resp.ctf_team &&
		targ != attacker && ((int)dmflags->value & DF_ARMOR_PROTECT)) {
		psave = asave = 0;
	} else {
//ZOID
		psave = CheckPowerArmor (targ, point, normal, take, dflags);
		take -= psave;
	
		asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
		take -= asave;
	}

	//treat cheat/powerup savings the same as armor
	asave += save;

//ZOID
//resistance tech
	take = CTFApplyResistance(targ, take);
//ZOID

	// team damage avoidance
	if (!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage (targ, attacker))
		return;

//ZOID
	CTFCheckHurtCarrier(targ, attacker);
//ZOID

// ROGUE - this option will do damage both to the armor and person. originally for DPU rounds
	if (dflags & DAMAGE_DESTROY_ARMOR)
	{
		if(!(targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) &&
		   !(client && client->invincible_framenum > level.framenum))
		{
			take = damage;
		}
	}
// ROGUE

// do the damage
	if (take)
	{
	//PGM		need more blood for chainfist.
		if(targ->flags & FL_MECHANICAL)
		{
			SpawnDamage ( TE_ELECTRIC_SPARKS, point, normal, take, targ->flags);
		}
		else if ((targ->svflags & SVF_MONSTER) || (client))
		{
			int h;

			if (take < 25)
				h = TE_BLOOD;
			else if (take < 50)
				h = TE_BLOOD;
			else if (take < 75)
				h = TE_MOREBLOOD;
			else
				h = TE_MOREBLOOD;

			SpawnDamage (h, point, normal, take, targ->flags);
		}
		else
			SpawnDamage (te_sparks, point, normal, take, targ->flags);


		targ->health = targ->health - take;
			
		if (targ->health <= 0)
		{
			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
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
		client->damage_parmor += psave;
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
		{
			continue;
		}
		if (!ent->takedamage)
		{
			continue;
		}
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
				if (Q_stricmp(inflictor->classname, "gravity_grenade") == 0)
				{
					ent->gravity_lift = level.time + 10.0;
					if (ent->client && attacker->client)
						safe_cprintf (ent, PRINT_HIGH, "Your gravitonation field was modified by %s!\n", attacker->client->pers.netname);
				}

				if (Q_stricmp(inflictor->classname, "freeze") == 0)
				{
					if (!ent->frozen)
						freeze_player(ent);
				}

				if ((Q_stricmp(inflictor->classname, "freeze") == 0) || (Q_stricmp(inflictor->classname, "gravity_grenade") == 0))
					continue;
					//gi.dprintf ("Ent takedamage\n");
					T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
		
				if (damage == 1000)
					ent->flags |= FL_NOGIB;
			}
		}
	}
}


void T_RadiusDamage_Pos (vec3_t position, edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, position, radius)) != NULL)
	{
		if (ent == ignore)
		{
			continue;
		}
		if (!ent->takedamage)
		{
			continue;
		}
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
				if (Q_stricmp(inflictor->classname, "gravity_grenade") == 0)
				{
					ent->gravity_lift = level.time + 10.0;
					if (ent->client && attacker->client)
						safe_cprintf (ent, PRINT_HIGH, "Your gravitonation field was modified by %s!\n", attacker->client->pers.netname);
				}

				if (Q_stricmp(inflictor->classname, "freeze") == 0)
				{
					if (!ent->frozen)
						freeze_player(ent);
				}

				if ((Q_stricmp(inflictor->classname, "freeze") == 0) || (Q_stricmp(inflictor->classname, "gravity_grenade") == 0))
					continue;
					//gi.dprintf ("Ent takedamage\n");
					T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
			}
		}
	}
}
