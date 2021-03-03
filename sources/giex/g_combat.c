// g_combat.c

#include "g_local.h"
#define RESBOOT_MAGUSE_MULT 0.8
#define MAGARMOR_MAGUSE_MULT 2.5

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
void monster_corpse_think(edict_t *ent);
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
//	if ((targ->client) && (targ->health < -999))
//		targ->health = -999;

	if (!(targ->svflags & SVF_MONSTER) || (attacker->client))
		targ->enemy = attacker;

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD)) {
		if (targ->monsterinfo.ability & GIEX_MABILITY_STEALTH) {
			removeStealth();
		}
		targ->svflags |= SVF_DEADMONSTER;	// now treat as a different content type
		if (!(targ->monsterinfo.aiflags & AI_GOOD_GUY)) {
			level.killed_monsters++;

			if (coop->value && (level.killed_monsters == level.total_monsters)) {
				gi.bprintf(PRINT_HIGH, "All monsters killed, %d bonus exp awarded!\n", 1000 + 100 * level.total_monsters);
				giveExpToAll(1000 + 100 * level.total_monsters);
			}
//			if (coop->value && attacker->client)
//				attacker->client->resp.score++;
			// medics won't heal monsters that they kill themselves
			if (attacker->classid == CI_M_MEDIC)
				targ->owner = attacker;
		}
	}

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD)) {
		targ->touch = NULL;
		monster_death_use (targ);
	}
	if ((targ->classid != CI_CORPSESPORE) && (targ->classid != CI_GIBRAIN) && (targ->classid != CI_PIZZAGIB) && (targ->classid, CI_M_INSANE) && (targ->deadflag != DEAD_DEAD)) { // Insane should never spawn ammo or powerups
//		gi.dprintf("Calling ammo and powerup spawn for %s\n", targ->classname);

		if ((game.monsterhunt == 10) && (targ->svflags & SVF_MONSTER)) {
			if (targ->classid != CI_M_JORG) { // You don't get any from Jorg...
				int i, num_players = 0;
				edict_t *ent;

				for (i=0 ; i<maxclients->value ; i++) {
					ent = g_edicts + 1 + i;
					if (!ent->inuse)
						continue;
					if (!ent->client->pers.loggedin)
						continue;
					num_players++;
				}
				if (num_players > 0) {
					for (i = 0; i < num_players; i++) {
						spawnAmmo(targ, attacker);
						spawnItem(targ, attacker);
						spawnPowerup(targ, attacker);
					}
				}
			}
		} else {
			spawnAmmo(targ, attacker);
			spawnItem(targ, attacker);
			spawnPowerup(targ, attacker);
		}

		if (targ->client) {
			if (attacker->client) {
				if (targ->radius_dmg > attacker->radius_dmg) {
					targ->client->pers.skills.stats[GIEX_STAT_LOWER_PLAYER_DEATHS]++;
					attacker->client->pers.skills.stats[GIEX_STAT_HIGHER_PLAYER_KILLS]++;
				} else {
					targ->client->pers.skills.stats[GIEX_STAT_HIGHER_PLAYER_DEATHS]++;
					attacker->client->pers.skills.stats[GIEX_STAT_LOWER_PLAYER_KILLS]++;
				}
			} else if (attacker->svflags & SVF_MONSTER) {
				targ->client->pers.skills.stats[GIEX_STAT_MONSTER_DEATHS]++;
			} else {
				targ->client->pers.skills.stats[GIEX_STAT_OTHER_DEATHS]++;
			}
		} else if ((targ->svflags & SVF_MONSTER) && (attacker->client)) {
			if (targ->monsterinfo.skill < 8) {
				attacker->client->pers.skills.stats[GIEX_STAT_LOW_MONSTER_KILLS]++;
			} else if (targ->monsterinfo.skill < 16) {
				attacker->client->pers.skills.stats[GIEX_STAT_MED_MONSTER_KILLS]++;
			} else if (targ->monsterinfo.skill < 24) {
				attacker->client->pers.skills.stats[GIEX_STAT_HI_MONSTER_KILLS]++;
			} else {
				attacker->client->pers.skills.stats[GIEX_STAT_VHI_MONSTER_KILLS]++;
			}
		}
	}

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
static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags) {
	gclient_t	*client;
	float pierce = 0;
	int			save;
	int			power_armor_type;
	int			damagePerCell;
	int			pa_te_type;
	int			power;
	int			power_used;
	int			index = 0;

	if (!damage)
		return 0;

	client = ent->client;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

/*	if (client) {
		power_armor_type = PowerArmorType (ent);
		if (power_armor_type != POWER_ARMOR_NONE) {
			index = ITEM_INDEX(FindItem("Cells"));
			power = client->pers.inventory[index] - (ent->client->pers.max_cells - 200);
		}
	} else*/ if (ent->svflags & SVF_MONSTER) {
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
	} else {
		return 0;
	}

	if (power_armor_type == POWER_ARMOR_NONE)
		return 0;
	if (!power)
		return 0;

	if (dflags & DAMAGE_25_PIERCE) {
		pierce += 0.25;
	} else if (dflags & DAMAGE_50_PIERCE) {
		pierce += 0.5;
	} else if (dflags & DAMAGE_75_PIERCE) {
		pierce += 0.75;
	}

	damage *= (1 - pierce);

	if (power_armor_type == POWER_ARMOR_SCREEN) {
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

		damagePerCell = 2;
		pa_te_type = TE_SCREEN_SPARKS;
		damage = (3 * damage) / 4;
	} else {
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

static int CheckArmor (edict_t *ent, edict_t *attacker, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags, int mod, qboolean mod_magic) {
	gclient_t	*client;
	float armor_save = 0;
	float pierce = 0;
	float breaker = 1.0;
	int			save;
	float		armoraff;
	int			armoraffsave;
	int			index;
	iteminfo_t *ainfo = NULL;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;
	ainfo = getWornItemInfo(ent, 1);

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	if (ent->health <= 0)
		return 0;

	index = ITEM_INDEX(FindItem("Body Armor"));
	if (!index)
		return 0;

	if (attacker->client && attacker->client->pers.skill[2] > 0) { // Armor piercing
		int item = getWornItem(attacker, GIEX_ITEM_WEAPON);
		if (mod == MOD_BLASTER) {
			iteminfo_t *winfo = getWornItemInfo(attacker, GIEX_ITEM_WEAPON);
			pierce = getBlasterPierce(attacker, item, winfo);//winfo->arg4 + winfo->arg5 * attacker->client->pers.skill[2];
		} else if (!mod_magic) {
			pierce = getWeaponPierce(attacker, item);
		}
	}
	if (attacker->client && (attacker->client->pers.skill[71] > 0) && (!mod_magic)) { // Armor breaker (extra damage to armor)
		breaker += 0.05 * attacker->client->pers.skill[71];
	}

	if (dflags & DAMAGE_25_PIERCE) {
		pierce += 0.25;
	} else if (dflags & DAMAGE_50_PIERCE) {
		pierce += 0.5;
	} else if (dflags & DAMAGE_75_PIERCE) {
		pierce += 0.75;
	} else if (dflags & DAMAGE_100_PIERCE) {
		pierce += 1.0;
	}

	if ((attacker->svflags & SVF_MONSTER) && (attacker->monsterinfo.ability & GIEX_MABILITY_ARMORPIERCE)) {
		pierce += 0.5;
	}

	armor_save = ainfo->arg3 + ainfo->arg4 * client->pers.skill[23]/* + 0.002 * pow(ent->client->pers.skills.classLevel[1], 1.2)*/ - pierce;
	if (armor_save > 1)
		armor_save = 1;
	if (armor_save <= 0)
		return 0;

	save = ceil(armor_save * damage * breaker);
	if (save < 1) {
		return 0;
	}

	armoraff = 0.015 * client->pers.skill[44];
	if (armoraff > 0.8)
		armoraff = 0.8;
	armoraffsave = (int) ceil(save * armoraff);
	if (armoraffsave > 0) {
		if (armoraff > 0.5) {
			if ((client->silencer_shots > 0) && (client->pers.skill[58] > 5)) {
				client->silencer_shots -= 1;
			} else {
				gi.sound(ent, CHAN_ITEM, gi.soundindex("giex/magarm2.wav"), 1, ATTN_NORM, 0);
			}
		} else if (armoraff > 0.25) {
			if ((client->silencer_shots > 0) && (client->pers.skill[58] > 5)) {
				client->silencer_shots -= 1;
			} else {
				gi.sound(ent, CHAN_ITEM, gi.soundindex("giex/magarm1.wav"), 1, ATTN_NORM, 0);
			}
		}
	}

	save -= armoraffsave;
	if (save >= client->pers.inventory[index])
		save = client->pers.inventory[index];

	if (!save) {
		if (armoraffsave > 0)
			return 1;
		return 0;
	}
	client->pers.inventory[index] -= save;
	SpawnDamage (te_sparks, point, normal, save);

	return (int) ((save + armoraffsave) / breaker);
}

void M_ReactToDamage (edict_t *targ, edict_t *attacker) {
	int i;
	edict_t *ent;

//	gi.dprintf("%s %s %s\n", targ->classname, targ->enemy ? targ->enemy->classname : "None", attacker->classname);
	if (targ->nextthink > level.time + 2 * FRAMETIME) {
		targ->nextthink = level.time + FRAMETIME;
		targ->monsterinfo.run(targ);
	}
	if (targ->monsterinfo.pausetime > level.time + 20) {
		targ->monsterinfo.pausetime = level.time + FRAMETIME;
	}

	if (!(attacker->client) && !(attacker->svflags & SVF_MONSTER))
		return;

	if (attacker == targ || attacker == targ->enemy)
		return;

	// if we are a good guy monster and our attacker is a player
	// or another good guy, do not get mad at them
	if (targ->monsterinfo.aiflags & AI_GOOD_GUY) {
		if (attacker->client || (attacker->monsterinfo.aiflags & AI_GOOD_GUY))
			return;
	}

//Never switch targets if we have a player as enemy and it is in sight
	if (targ->enemy && targ->enemy->client && visible(targ, targ->enemy)) {
		if (attacker != targ->enemy)
			targ->oldenemy = attacker;
//		FoundTarget (targ);
		return;
	}
	// we now know that we are not both good guys

	// Monster have no enemies (first time it is attacked), all monsters of same type within view will help against attacker
	if (((!targ->enemy) || (targ->monsterinfo.aiflags & AI_SOUND_TARGET)) && (game.craze != 10) && (game.monsterhunt != 10)) {
		ent = &g_edicts[0];
		for (i=0 ; i<globals.num_edicts ; i++, ent++) {
			if (!ent->inuse)
				continue;
			if (!(ent->svflags & SVF_MONSTER))
				continue;
			if (targ == ent)
				continue;
			if (ent->classid != targ->classid)
				continue;
			if (ent->health < 1)
				continue;
			if (attacker->client && (attacker->client->silencer_shots > 0) && (random() < 0.09 * attacker->client->pers.skill[58]))
				continue;
			if (!visible(ent, targ))
				continue;
			if (ent->enemy && !(ent->monsterinfo.aiflags & AI_SOUND_TARGET) && visible(ent, ent->enemy))
				continue;
			ent->enemy = attacker;
			if (!(ent->monsterinfo.aiflags & AI_DUCKED))
				FoundTarget (ent);
		}
	}

	if (attacker->client) {
		targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;

		if (targ->enemy && targ->enemy->client) {
			targ->oldenemy = targ->enemy;
		}
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
		return;
	}

//Attacker is a monster

//Medics will never attack other monsters
	if (targ->classid == CI_M_MEDIC)
		return;

//We're attacking the same enemy
	if (targ->enemy && (targ->enemy == attacker->enemy)) {
		targ->oldenemy = attacker;
		return;
	}

/*
	if (((targ->flags & (FL_FLY|FL_SWIM)) == (attacker->flags & (FL_FLY|FL_SWIM)))
		(strcmp (targ->classname, attacker->classname) != 0) &&
		(!targ->monsterinfo.aggressive) &&
		(strcmp(attacker->classname, "monster_tank") != 0) &&
		(strcmp(attacker->classname, "monster_supertank") != 0) &&
		(strcmp(attacker->classname, "monster_makron") != 0) &&
		(strcmp(attacker->classname, "monster_jorg") != 0) )
	{*/
// If currently we don't have a target, we're chasing a sound, or we don't see our target, attack the monster shooting us
	if (!targ->enemy || (targ->monsterinfo.aiflags & AI_SOUND_TARGET) || !visible(targ, targ->enemy)) {
		targ->monsterinfo.aiflags &= ~AI_SOUND_TARGET;
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
		return;
	}
//	}
	// if they *meant* to shoot us, then shoot back
	if (attacker->enemy == targ) {
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}
	// otherwise get mad at whoever they are mad at (help our buddy) unless it is us!
/*	else if (attacker->enemy && attacker->enemy != targ) {
		if (targ->enemy && targ->enemy->client)
			targ->oldenemy = targ->enemy;
		targ->enemy = attacker->enemy;
		if (!(targ->monsterinfo.aiflags & AI_DUCKED))
			FoundTarget (targ);
	}*/
}

qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker) {
		//FIXME make the next line real and uncomment this block
		// if ((ability to damage a teammate == OFF) && (targ's team == attacker's team))
	return false;
}

int T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod) {
	qboolean mod_magic = false;
	gclient_t	*client;
	int			take = 0;
	int			save = 0;
	int			asave = 0;
	int			psave = 0;
	int			rsave = 0;
	int			sasave = 0;
	int			te_sparks;

	if (!targ->takedamage)
		return 0;

// Can't heal monsters
	if ((damage < 0) && (targ->svflags & SVF_MONSTER)) {
		return 0;
	}

	if (mod & MOD_MAGIC) { //damage dealt by magic
		mod_magic = true;
		mod &= ~MOD_MAGIC;
	}
	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs

	// team damage avoidance, if not telefrag
	if (mod != MOD_TELEFRAG) {
		if (deathmatch->value && attacker->client && targ->client && (targ != attacker) && (((targ->count == attacker->count) && teams->value) || (game.monsterhunt == 10)) && (damage > 0)) { // Two players on same team
			damage *= teamdamage->value;
		}
		if (coop->value && attacker->client && targ->client && (targ != attacker)) {
			damage *= teamdamage->value;
		}
	}
	meansOfDeath = mod;

	client = targ->client;

	if (dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	VectorNormalize(dir);

	if (targ->flags & FL_NO_KNOCKBACK)
		knockback = 0;

// Simulate armor pierce and armor breaker on monsters
// This has been disabled, since it applies bonus AFTER all other bonuses, resulting in crazy total bonuses
/*	if ((targ->svflags & SVF_MONSTER) && (attacker->client) && (!mod_magic)) {
		if (attacker->client->pers.skill[2] > 0) {
			if (mod == MOD_BLASTER) {
				iteminfo_t *winfo = getWornItemInfo(attacker, 0);
				damage *= 1 + 0.75 * (winfo->arg4 + winfo->arg5 * attacker->client->pers.skill[2]);
			} else {
				damage *= 1 + 0.02 * attacker->client->pers.skill[2];
			}
		}
		if (attacker->client->pers.skill[72] > 0) {
			damage *= 1 + 0.02 * attacker->client->pers.skill[72];
		}
	}*/

	if ((targ->svflags & SVF_MONSTER) && (targ->health > 0)) {
		float manaburn_mult = 0.0f;
		float manaleech_mult = 0.0f;
		if ((targ->radius_dmg) && (targ->monsterinfo.ability) && (damage > 0)) {
			rsave = 0;
			if ((targ->monsterinfo.ability & GIEX_MABILITY_RES_EXPL) &&
				((mod == MOD_GRENADE) || (mod == MOD_G_SPLASH) ||
				(mod == MOD_ROCKET) || (mod == MOD_R_SPLASH) ||
				(mod == MOD_HANDGRENADE) || (mod == MOD_HG_SPLASH))) {
				rsave += 0.33 * damage;
			}
			if ((targ->monsterinfo.ability & GIEX_MABILITY_RES_IMPACT) &&
				((mod == MOD_MACHINEGUN) || (mod == MOD_CHAINGUN) ||
				(mod == MOD_SHOTGUN) || (mod == MOD_SSHOTGUN))) {
				rsave += 0.33 * damage;
			}
			if ((targ->monsterinfo.ability & GIEX_MABILITY_RES_ENERGY) &&
				((mod == MOD_BLASTER) || (mod == MOD_HYPERBLASTER) ||
				(mod == MOD_LASERMINE) || (mod == MOD_RAILGUN) ||
				(mod == MOD_BFG_LASER) || (mod == MOD_BFG_BLAST) || (mod == MOD_BFG_EFFECT))) {
				rsave += 0.33 * damage;
			}
			if ((targ->monsterinfo.ability & GIEX_MABILITY_RES_BMAGIC) &&
				((mod == MOD_PLAGUEBOMB) || (mod == MOD_DRAIN) || (mod == MOD_SPORE))) {
				rsave += 0.33 * damage;
			}
			if ((targ->monsterinfo.ability & GIEX_MABILITY_RES_FMAGIC) &&
				((mod == MOD_INFERNO) || (mod == MOD_FIREBOLT) ||
				(mod == MOD_FIREBALL) || (mod == MOD_CORPSEEXPLOSION))) {
				rsave += 0.33 * damage;
			}
			if ((targ->monsterinfo.ability & GIEX_MABILITY_RES_LMAGIC) &&
				((mod == MOD_LIGHTNING) || (mod == MOD_SPARK) || (mod == MOD_BOLT) || (mod == MOD_STORM))) {
				rsave += 0.33 * damage;
			}
			if (rsave > 0) {
				//Anti-resist
				int arlvl = getAuraLevel(attacker, 84);
				if (arlvl > 0) {
					int slot = getAuraSlot(attacker, 84);
					if (attacker->client->aura_caster[slot]->client->magic > 0) {
						attacker->client->aura_caster[slot]->client->magic -= 0.1 * rsave;
						attacker->client->aura_caster[slot]->client->magregentime = level.time + 1.0;
						rsave *= 1 - (arlvl * 0.02);
					}
				}
				damage -= rsave;
			}
		}

		if (attacker->client) {
			if (dflags & DAMAGE_10_MANABURN) // 10%
				manaburn_mult += 0.1;
			if (dflags & DAMAGE_25_MANABURN) // 25%
				manaburn_mult += 0.25;
			if (dflags & DAMAGE_50_MANABURN) // 50%
				manaburn_mult += 0.5;
			if (dflags & DAMAGE_100_MANABURN) // 100%
				manaburn_mult += 1.0;
			if (dflags & DAMAGE_200_MANABURN) // 200%
				manaburn_mult += 2.0;
			if (dflags & DAMAGE_400_MANABURN) // 400%
				manaburn_mult += 4.0;
			if (dflags & DAMAGE_50_MANALEECH) // 50%
				manaleech_mult += 0.5;
			if (dflags & DAMAGE_100_MANALEECH) // 100%
				manaleech_mult += 1.0;
			if (dflags & DAMAGE_200_MANALEECH) // 200%
				manaleech_mult += 2.0;

			if ((manaburn_mult > 0.0) && (attacker->client->magic < attacker->client->max_magic * 4)) {
				attacker->client->magic += (int) ceil(damage * manaburn_mult);
				if (attacker->client->magic > attacker->client->max_magic * 4) {
					attacker->client->magic = attacker->client->max_magic * 4;
				}
				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (targ-g_edicts);
				gi.WriteByte (MZ_NUKE4);
				gi.multicast (targ->s.origin, MULTICAST_PVS);
			}
			if (manaleech_mult > 0.0) {
				if (attacker->client->magic < attacker->client->max_magic * 4) {
					attacker->client->magic += (int) ceil(damage * manaleech_mult);
					if (attacker->client->magic > attacker->client->max_magic * 4) {
						attacker->client->magic = attacker->client->max_magic * 4;
					}
				}
				if (manaburn_mult == 0.0) {
					gi.WriteByte (svc_muzzleflash);
					gi.WriteShort (targ-g_edicts);
					gi.WriteByte (MZ_NUKE4);
					gi.multicast (targ->s.origin, MULTICAST_PVS);
				}
			}
		}
	}

	if (client && (damage > 0) && (targ->health > 0)) {
		int pre_damage = damage;
		int nosanc_damage = damage;
		int pre_magic = targ->client->magic;
		float res_boost = 0.15 * targ->client->pers.skill[45]; //Resistance boost
		float manaburn_mult = 0.0f;
		float manaleech_mult = 0.0f;
		if (targ->client->magic <= 0)
			res_boost = 0;

//Sanctuary aura
		if ((getAuraLevel(targ, 65) > 0) && (damage > 0)) {
			int slot = getAuraSlot(targ, 65);
			if (targ->client->aura_caster[slot]->client->magic > 0) {
				float bonus = getMagicBonuses(targ->client->aura_caster[slot], 65);
				float dmg_mult = 1.0;
				float cost_mult = 0.0;

				dmg_mult = (0.004 * targ->client->aura_level[slot] * (0.2 + bonus * 0.8));
				if (dmg_mult > 1)
					dmg_mult = 1.0;

				cost_mult = 0.002 * targ->client->aura_level[slot];
				if (targ->client->aura_level[slot] > 40) {
					cost_mult += 0.002 * (targ->client->aura_level[slot] - 40);
				}
				cost_mult *= ((float) (targ->client->aura_caster[slot]->client->max_magic + 800.0)) / 800.0;

				if (cost_mult * damage > targ->client->aura_caster[slot]->client->magic) {
					int dmgsave = (int) targ->client->aura_caster[slot]->client->magic / cost_mult;
					//gi.dprintf("%d %d %f %d %d\n", damage, dmgsave, cost_mult, targ->client->aura_caster[slot]->client->magic, (int) dmgsave * cost_mult);
					sasave += dmgsave * dmg_mult;
					damage -= dmgsave * dmg_mult;
					targ->client->aura_caster[slot]->client->magic -= dmgsave * cost_mult;
				} else {
					targ->client->aura_caster[slot]->client->magic -= cost_mult * damage;
					sasave += dmg_mult * damage;
					damage *= 1.0 - dmg_mult;
				}
				targ->client->aura_caster[slot]->client->magregentime = level.time + 1.0;
				gi.sound(targ, CHAN_ITEM, gi.soundindex("giex/magarm1.wav"), 0.5, ATTN_NORM, 0);
			}
		}

		rsave = 0;
		// Global (Damage resist)
		if (targ->client->pers.skill[46] > 0) {
			rsave += damage * 0.01 * targ->client->pers.skill[46];
		}

		// Bullet
		if ((targ->client->pers.skill[36] > 0) &&
			((mod == MOD_MACHINEGUN) || (mod == MOD_CHAINGUN))) {
			rsave += damage * 0.015 * targ->client->pers.skill[36] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// Pellet
		else if ((targ->client->pers.skill[37] > 0) &&
			((mod == MOD_SHOTGUN) || (mod == MOD_SSHOTGUN))) {
			rsave += damage * 0.015 * targ->client->pers.skill[37] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// Explosion
		else if ((targ->client->pers.skill[38] > 0) &&
			((mod == MOD_GRENADE) || (mod == MOD_G_SPLASH) ||
			(mod == MOD_ROCKET) || (mod == MOD_R_SPLASH) ||
			(mod == MOD_HANDGRENADE) || (mod == MOD_HG_SPLASH))) {
			rsave += damage * 0.015 * targ->client->pers.skill[38] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// Energy
		else if ((targ->client->pers.skill[39] > 0) &&
			((mod == MOD_BLASTER) || (mod == MOD_HYPERBLASTER) ||
			(mod == MOD_LASERMINE))) {
			rsave += damage * 0.015 * targ->client->pers.skill[39] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// High energy
		else if ((targ->client->pers.skill[40] > 0) &&
			((mod == MOD_RAILGUN) || (mod == MOD_BFG_LASER) ||
			(mod == MOD_BFG_BLAST) || (mod == MOD_BFG_EFFECT))) {
			rsave += damage * 0.015 * targ->client->pers.skill[40] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// World and hit damage
		else if (targ->client->pers.skill[41] > 0) {
			if ((mod == MOD_WATER) || (mod == MOD_SLIME) || (mod == MOD_FALLING) ||
				(mod == MOD_LAVA) || (mod == MOD_CRUSH)) {
				rsave += damage * 0.1 * targ->client->pers.skill[41];
			} else if (mod == MOD_HIT) {
				rsave += damage * 0.015 * targ->client->pers.skill[41] * res_boost;
				targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
			}
		}
		// Blood magic
		else if ((targ->client->pers.skill[42] > 0) &&
			((mod == MOD_PLAGUEBOMB) || (mod == MOD_DRAIN) || (mod == MOD_SPORE))) {
			rsave += damage * 0.015 * targ->client->pers.skill[42] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// Fire magic
		else if ((targ->client->pers.skill[43] > 0) &&
			((mod == MOD_INFERNO) || (mod == MOD_FIREBOLT) || (mod == MOD_FIREBALL) || (mod == MOD_CORPSEEXPLOSION))) {
			rsave += damage * 0.015 * targ->client->pers.skill[43] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		// Lightning magic
		else if ((targ->client->pers.skill[69] > 0) &&
			((mod == MOD_LIGHTNING) || (mod == MOD_SPARK) || (mod == MOD_BOLT))) {
			rsave += damage * 0.015 * targ->client->pers.skill[69] * res_boost;
			targ->client->magic -= RESBOOT_MAGUSE_MULT * ceil((pre_damage - damage) * res_boost);
		}
		if (rsave > 0) {
			//Anti-resist
			int arlvl = getAuraLevel(attacker, 84);
			if (arlvl > 0) {
				int slot = getAuraSlot(attacker, 84);
				if (attacker->client->aura_caster[slot]->client->magic > 0) {
					attacker->client->aura_caster[slot]->client->magic -= 0.1 * rsave;
					attacker->client->aura_caster[slot]->client->magregentime = level.time + 1.0;
					rsave *= 1 - (arlvl * 0.02);
				}
			}
			damage -= rsave;
			nosanc_damage -= rsave;
		}

		if (dflags & DAMAGE_10_MANABURN) // 10%
			manaburn_mult += 0.1;
		if (dflags & DAMAGE_25_MANABURN) // 25%
			manaburn_mult += 0.25;
		if (dflags & DAMAGE_50_MANABURN) // 50%
			manaburn_mult += 0.5;
		if (dflags & DAMAGE_100_MANABURN) // 100%
			manaburn_mult += 1.0;
		if (dflags & DAMAGE_200_MANABURN) // 200%
			manaburn_mult += 2.0;
		if (dflags & DAMAGE_400_MANABURN) // 400%
			manaburn_mult += 4.0;
		if (dflags & DAMAGE_50_MANALEECH) // 50%
			manaleech_mult += 0.5;
		if (dflags & DAMAGE_100_MANALEECH) // 100%
			manaleech_mult += 1.0;
		if (dflags & DAMAGE_200_MANALEECH) // 200%
			manaleech_mult += 2.0;

		if ((attacker->svflags & SVF_MONSTER) && (attacker->monsterinfo.ability & GIEX_MABILITY_MANABURN)) {
			manaburn_mult += 1.0;
		}

		if (manaburn_mult > 0.0) {
			targ->client->magic -= nosanc_damage * manaburn_mult;
			if (targ->client->magic < 0)
				targ->client->magic = 0;
			if ((attacker->client) && (attacker->client->magic < attacker->client->max_magic * 4)) {
				attacker->client->magic += nosanc_damage * manaburn_mult;
				if (attacker->client->magic > attacker->client->max_magic * 4) {
					attacker->client->magic = attacker->client->max_magic * 4;
				}
			}
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (targ-g_edicts);
			gi.WriteByte (MZ_NUKE4);
			gi.multicast (targ->s.origin, MULTICAST_PVS);
		}
		if (manaleech_mult > 0.0) {
			if (attacker->client->magic < attacker->client->max_magic * 4) {
				attacker->client->magic += (int) ceil(nosanc_damage * manaleech_mult);
				if (attacker->client->magic > attacker->client->max_magic * 4) {
					attacker->client->magic = attacker->client->max_magic * 4;
				}
			}
			if (manaburn_mult == 0.0) {
				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (targ-g_edicts);
				gi.WriteByte (MZ_NUKE4);
				gi.multicast (targ->s.origin, MULTICAST_PVS);
			}
		}

		if ((pre_magic != targ->client->magic) && (targ->client->magregentime < level.time + 1.0)) {
			targ->client->magregentime = level.time + 1.0;
		}

		if (damage < 0) {
			damage = 0;
		}
	}

// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK)) {
		if ((knockback) && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) && (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP)) {
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

	// check for godmode
	if ((targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) && (damage > 0)) {
		take = 0;
		save = damage;
		SpawnDamage (te_sparks, point, normal, save);
	}

	// check for invincibility
	if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION) && (damage > 0)) {
		if (targ->pain_debounce_time < level.time) {
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}
		take = 0;
		save = damage;
	}

	if (take > 0) {
		if ((targ->client) && (targ->client->damage_time < level.time + 0.5 + take * 0.001)) {
			targ->client->damage_time = level.time + 0.5 + take * 0.001;
		}
		psave = CheckPowerArmor (targ, point, normal, take, dflags);
		take -= psave;

		asave = CheckArmor (targ, attacker, point, normal, take, te_sparks, dflags, mod, mod_magic);
		take -= asave;
	}

// do the damage
/*	if ((attacker->client) && (targ->svflags & SVF_MONSTER)) {
		if (take <= 0) {
			gi.dprintf("NO DAMAGE\n", take);
			_asm int 3;
		}
	}*/
	if (rsave > 0) {
		SpawnDamage (TE_SCREEN_SPARKS, point, normal, rsave);
	}
	if (sasave > 0) {
		SpawnDamage (TE_SHIELD_SPARKS, point, normal, sasave);
	}

	if ((take > 0) || ( (take < 0) && (targ->health < 2 * targ->max_health)) ) {
		targ->health -= take;
	}
	if ( (targ->health > 0) && ((take != 0) || (take + asave > 0)) ) {
		addExp(attacker, targ, take + asave); //Also give exp for killing armor
	}
	if (take != 0) {
		if ((targ->svflags & SVF_MONSTER) || (client))
			SpawnDamage (TE_BLOOD, point, normal, take);
		else
			SpawnDamage (te_sparks, point, normal, take);


		if ((attacker->client) && (mod != MOD_TELEFRAG)) /* && (targ->health > 0))*/ {
			if ((damage > 0) && (attacker->client->pers.skill[34] /*|| (attacker->client->pers.skills.classLevel[4] > 0)*/) && ((targ->health + take) > 0)) {
				int maxhealth = attacker->max_health * 0.75;
				maxhealth += 13.5 * pow(attacker->client->pers.skills.classLevel[4], 1.05);
				if (attacker->health < maxhealth) {
					float amount = 0;
					float mult = 0;

					//mult += 0.001 * pow(attacker->client->pers.skills.classLevel[4], 1.1); // Vampire class level bonus
					//mult *= 0.4 + 0.6 * ((float) attacker->client->pers.skills.classLevel[4] / (float) attacker->radius_dmg); // Penalty if not pure Vampire
					mult += 0.01 * attacker->client->pers.skill[34]; //From item powerups
					if (attacker->client->damage_time > level.time) {
						mult *= 0.25;
					}
					amount = take * mult;
					if ((mod == MOD_TELEFRAG) || (attacker == targ))
						amount = 0;
					attacker->client->pers.add_health += amount;
					if (attacker->health > maxhealth)
						attacker->health = maxhealth;
				}
			}
		}

		if (targ->health <= 0) {
			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
			Killed (targ, inflictor, attacker, take, point);
			return take;
		}
	}

	if (take > 0) {
		if (targ->svflags & SVF_MONSTER) {
			edict_t *oldenemy = targ->enemy;
			if (targ->radius_dmg && (targ != attacker) && (attacker->takedamage) && (targ->monsterinfo.ability & GIEX_MABILITY_SHARDARMOR) && targ->monsterinfo.shardtime < (level.time - 0.1)) {
				vec3_t aim, end;
				float mult = level.time - targ->monsterinfo.shardtime;
				if (mult > 1.7)
					mult = 1.7;
				mult += 0.2;
				targ->monsterinfo.shardtime = level.time + 0.2;

				VectorMA(attacker->s.origin, 0.3, attacker->velocity, end);
				end[2] += attacker->viewheight;
				VectorSubtract (end, point, aim);

				if (targ->monsterinfo.ability & GIEX_MABILITY_DAMAGE) // Damage ability shouldn't affect Shard armor, halve mult
					mult *= 0.5;

				monster_fire_blaster (targ, point, aim, (int) ceil((16 + 10 * targ->monsterinfo.skill) * mult), 2200, MZ2_SOLDIER_BLASTER_1, EF_BLASTER);
				gi.sound(targ, CHAN_AUTO, gi.soundindex("giex/magarm2.wav"), 0.8, ATTN_NORM, 0);
			}
			M_ReactToDamage (targ, attacker);
			if (!(targ->monsterinfo.aiflags & AI_DUCKED) && (take)) {
				targ->pain_debounce_time = level.time + 2 + 0.1 * targ->monsterinfo.skill;
				if (oldenemy != NULL)
					targ->pain (targ, attacker, knockback, take);
			}
		} else if (client) {
			if (!(targ->flags & FL_GODMODE) && (take))
				targ->pain (targ, attacker, knockback, take);
		} else if (take) {
			if (targ->pain)
				targ->pain (targ, attacker, knockback, take);
		}
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if (client) {
		client->damage_parmor += psave;
		client->damage_armor += (asave + save);
		client->damage_blood += take;
		client->damage_knockback += knockback;
		VectorCopy (point, client->damage_from);
	}
	return take;
}


/*
============
T_RadiusDamage
============
*/
int T_RadiusDamage (edict_t *target, edict_t *inflictor, edict_t *attacker, float damage, float mindamage, edict_t *ignore, float radius, qboolean dmgfalloff, int mod) {
	float	points;
	float mult = 1.0f;
	int	kickback;
	int	dmg;
	int	result = 0;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	if (ignore != attacker) {
		VectorAdd (attacker->mins, attacker->maxs, v);
		VectorMA (attacker->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		if (VectorLength(v) < radius * 0.5f) {
			mult = 0.35f;
		} else if (VectorLength(v) < radius * 0.5f) {
			mult = 0.75f;
		}
	}
	while ((ent = findradius(ent, inflictor->s.origin, radius)) != NULL) {
		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		if (dmgfalloff) {
			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (inflictor->s.origin, v, v);
			points = ((mindamage - damage) / radius) * VectorLength(v) + damage;
//			points = damage * ( (float)(radius - VectorLength (v)) / (float)radius);
		} else {
			points = damage;
		}
		points *= mult;

		if ((ent->svflags & SVF_MONSTER) && (ent == attacker) && ((points + 10) > ent->health))
			continue;

//		if (ent == attacker)
//			points = points * 0.5;
		if (points >= 1)
		{
			kickback = (int) points;
			if (CanDamage (ent, inflictor))
			{
				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				if (ent->svflags & SVF_MONSTER) {
					if ((mod & 1023) == MOD_INFERNO) {
						points *= 0.7;
					}
				}
				if (((mod & 1023) == MOD_PLAGUEBOMB) || ((mod & 1023) == MOD_SWARM) || (((mod & 1023) == MOD_INFERNO)) || (((mod & 1023) == MOD_INCENDIARY))) {
					kickback = 0;
				} else if ((mod & 1023) == MOD_FIREBALL) {
					kickback = (int) (points / 2.0);
				} else if ((mod & 1023) == MOD_CORPSEEXPLOSION) {
					kickback = (int) (points / 3.0);
				} else if ((mod & 1023) == MOD_TANKSTRIKE) {
					kickback = (int) 2 * points;
					if (ent->groundentity)
						kickback *= 4;
				}
				if (kickback > 200)
					kickback = 200;

				dmg = T_Damage(ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int) points, kickback, DAMAGE_RADIUS, mod);
				if (target == NULL) {
					result += dmg;
				} else if (ent == target) {
					result = dmg;
				}
			}
		}
	}
	return result;
}
