// g_combat.c

#include "g_local.h"
#include "damage.h"

//K03 Begin
//==============================================================
// Scale damage amount by location of hit on player's body..
//==============================================================
#define LEG_DAMAGE     (height/2.0)-abs(targ->mins[2])-3
#define STOMACH_DAMAGE (height/1.6)-abs(targ->mins[2])
#define CHEST_DAMAGE   (height/1.4)-abs(targ->mins[2])

//==============================================================
/*
float location_scaling(edict_t *targ, vec3_t point, int damage) {
	float z_rel, height;

	if (!G_EntExists(targ))
		return 1.0;

	if (!targ->client && targ->svflags & SVF_MONSTER)
		return 1.0;

	if (targ->flags & FL_GODMODE)
		return 1.0;

	height = abs(targ->mins[2])+targ->maxs[2];
	z_rel = point[2]-targ->s.origin[2];
	if (z_rel < LEG_DAMAGE)
	  return 0.60;  // Scale down by 2/5
	else if (z_rel < STOMACH_DAMAGE)
	  return 0.80;  // Scale down by 1/5
	else if (z_rel < CHEST_DAMAGE)
	  return 1.20;  // Scale up by 1/5
	else
	{
		if (targ->client && targ->client->pers.inventory[ITEM_INDEX(FindItem("Helmet"))])
			return 0.6;//scale down by 2/5
		return 2.40;// Scale up by 12/5
	}

	return 1.0; // keep damage the same..
}
*/

void TossClientWeapon (edict_t *self);


#define BitCmp(x,bit) ((int)(x)&(bit))

qboolean HitTheWeapon (edict_t *targ, edict_t *attacker, vec3_t point, int take, int dflags) 
{
	edict_t *cl_ent=attacker;
	float z_rel, height;

	if (PM_MonsterHasPilot(attacker))
		cl_ent = attacker->activator;

	if (!G_EntIsAlive(targ) || !G_EntIsAlive(cl_ent))
		return false;
	if (targ->knockweapon_debounce_time > level.time)
		return false; // don't knock too often!
	if (!cl_ent->client || !targ->client)
		return false;
	if (cl_ent->myskills.abilities[WEAPON_KNOCK].disable)
		return false;
	if (cl_ent->myskills.abilities[WEAPON_KNOCK].current_level < 1)
		return false;
	if (OnSameTeam(cl_ent, targ))
		return false;
	if (targ->client->invincible_framenum > level.framenum)
		return false;
	
	// some weapons can't be knocked!
	if (targ->client->pers.weapon == FindItem("Blaster")
		|| targ->client->pers.weapon == FindItem("Sword"))
		return false;

	// can't knock weapon from morphs or poltergeists
	if (targ->mtype || (isMorphingPolt(targ)))
		return false;

	if ((targ->health-take) > (0.5*targ->max_health))
		return false;
	
	targ->knockweapon_debounce_time = level.time + 1.0;
	// check for impact location
	// if it's at about the right height, and in front, then
	// we're probably on-target
	height = abs(targ->mins[2])+targ->maxs[2];
	z_rel = point[2]-targ->s.origin[2];

	//gi.dprintf("z_rel: %.1f point: %.1f origin: %.1f, chest: %.1f stomach: %.1f", 
	//	z_rel, point[2], targ->s.origin[2], CHEST_DAMAGE, STOMACH_DAMAGE);

	//if (infront(attacker, targ) && (z_rel >= STOMACH_DAMAGE) 
	//	&& (z_rel <= CHEST_DAMAGE) && (random() > 0.4))

	//4.2 relaxed weapon knock requirements; just hit them above the waistline
	if (infront(targ, attacker) && (z_rel >= 0) && (random() > 0.4))
	{
		gi.centerprintf(targ, "Your weapon was knocked!\n");
		targ->knockweapon_debounce_time = level.time + 2.0;
		return true;
	}

	return false;
}
//K03 End
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

void G_ApplyFury(edict_t *attacker)
{
	int chance = attacker->myskills.abilities[FURY].current_level;
	int r = GetRandom(0, 66);

	//The following is truncated so that every 4th point adds an extra 1%.
	if (r < chance)
	{
		float duration = FURY_DURATION_BASE + FURY_DURATION_BONUS * chance;

		if (attacker->client)
			safe_cprintf(attacker, PRINT_HIGH, "For the next %0.1f seconds you will become the fury.\n", duration);

		gi.sound(attacker, CHAN_AUTO, gi.soundindex("ctf/tech2x.wav"), 1, ATTN_NORM, 0);

		//Player got a fury boost!
		attacker->fury_time = level.time + duration;
		attacker->myskills.abilities[FURY].delay = attacker->fury_time + 1.0;
	}
	else
	{
		//They didn't get the boost. Try again later.
		attacker->myskills.abilities[FURY].delay = level.time + 1.0;
	}
}


/*
============
Killed
============
*/
void VortexAddExp(edict_t *attacker, edict_t *targ);
void drone_death (edict_t *self, edict_t *attacker);
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (targ->health < -999)
		targ->health = -999;

	if (targ->mtype != M_COCOON)// cocoon uses enemy after death to restore cocooned entity
		targ->enemy = attacker;

	//K03 Begin
	//Has the spreewar been broken?
	if (SPREE_WAR == true && targ == SPREE_DUDE)
	{
		SPREE_WAR = false;
		SPREE_DUDE = NULL;
	}
	//K03 End

	if (!targ->client && !targ->deadflag && (targ->solid != SOLID_NOT))
		VortexAddExp(attacker, targ);

	if (targ->movetype == MOVETYPE_PUSH || targ->movetype == MOVETYPE_STOP || targ->movetype == MOVETYPE_NONE)
	{	// doors, triggers, etc
		targ->die (targ, inflictor, attacker, damage, point);
		return;
	}

	if ((targ->svflags & SVF_MONSTER) && (targ->deadflag != DEAD_DEAD))
	{
		//targ->touch = NULL;
		drone_death (targ, attacker);
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

static int CheckShield (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	int		save, pa_te_type;
	edict_t *cl_ent = NULL;

	if (!damage)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;
	if (dflags & DAMAGE_NO_ABILITIES)
		return 0;
	if (ent->deadflag == DEAD_DEAD)
		return 0;

	//4.2 check for player-monster, assign cl_ent to always point to the client (if there is one)
	if (PM_MonsterHasPilot(ent))
		cl_ent = ent->activator;
	else if (ent->client)
		cl_ent = ent;
	else
		return 0;//non-players are not supported

	// flag carriers can't use power armor in CTF
	if (cl_ent && ctf->value && ctf_enable_balanced_fc->value && HasFlag(cl_ent))
		return 0;
	
	// shield is not activated
	if (!cl_ent->shield || (cl_ent->shield_activate_time > level.time))
		return 0;

	if (cl_ent->shield == 1)
	{
		vec3_t		forward, v;

		// only works if damage point is in front
		AngleVectors(ent->s.angles, forward, NULL, NULL);
		VectorSubtract(point, ent->s.origin, v);
		VectorNormalize(v);
		if (DotProduct (v, forward) <= 0.3)
			return 0;

		pa_te_type = TE_SCREEN_SPARKS;
		save = SHIELD_FRONT_PROTECTION * damage;// absorb some damage
	}
	else
	{
		pa_te_type = TE_SHIELD_SPARKS;
		save = SHIELD_BODY_PROTECTION * damage;
	}
	
	SpawnDamage (pa_te_type, point, normal, save);

	//gi.dprintf("damage = %d, save = %d\n", damage, save);

	return save;
}

static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags)
{
	gclient_t	*client;
	int			save=0; // max absorbtion
	int			power_armor_type;
	int			index;
	float		damagePerCell; //GHz
	int			pa_te_type;
	int			power; // cells
	int			power_used; // cells used
	edict_t		*cl_ent=NULL;

	//4.2 if shield absorbed all the damage, we're done
	if ((save = CheckShield(ent, point, normal, damage, dflags)) == damage)
		return save;

	if (!damage)
		return 0;

	client = ent->client;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;
	if (dflags & DAMAGE_NO_ABILITIES)
		return 0;
	if (ent->deadflag == DEAD_DEAD)
		return 0;

	//4.2 check for player-monster, assign cl_ent to always point to the client (if there is one)
	if (PM_MonsterHasPilot(ent))
		cl_ent = ent->activator;
	else if (client)
		cl_ent = ent;

	// flag carriers can't use power armor in CTF
	if (cl_ent && ctf->value && ctf_enable_balanced_fc->value && HasFlag(cl_ent))
		return 0;

	if (cl_ent)
	{
		power_armor_type = PowerArmorType (cl_ent);
		if (power_armor_type != POWER_ARMOR_NONE)
		{
			index = ITEM_INDEX(Fdi_CELLS);
			power = cl_ent->client->pers.inventory[index];
		}
	}
	else if (ent->svflags & SVF_MONSTER)
	{
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
	}
	else if (ent->creator && ent->creator->client)
	{
		power_armor_type = ent->monsterinfo.power_armor_type;
		power = ent->monsterinfo.power_armor_power;
	}
	else
		return 0;

	if (power_armor_type == POWER_ARMOR_NONE)
		return save;
	if (!power)
		return save;

	if (power_armor_type == POWER_ARMOR_SCREEN)
	{
		int			pslevel;
		vec3_t		vec;
		float		dot;
		vec3_t		forward;		

		// only works if damage point is in front
		AngleVectors (ent->s.angles, forward, NULL, NULL);
		VectorSubtract (point, ent->s.origin, vec);
		VectorNormalize (vec);
		dot = DotProduct (vec, forward);
		if (dot <= 0.3)
			return save;
		
		if (cl_ent)
		{
			// use power shield level or brain level, whichever is highest
			pslevel = cl_ent->myskills.abilities[POWER_SHIELD].current_level;
			if (cl_ent->mtype == MORPH_BRAIN && cl_ent->myskills.abilities[BRAIN].current_level > pslevel)
				pslevel = cl_ent->myskills.abilities[BRAIN].current_level;
			
			if (pslevel > 0)
				damagePerCell = 0.1 + 0.19 * pslevel;
			else
				damagePerCell = 1.0;
		}
		else	
			damagePerCell = 1.0;

		pa_te_type = TE_SCREEN_SPARKS;
	}
	else
	{
		damagePerCell = 1.0;
		pa_te_type = TE_SHIELD_SPARKS;
		damage *= 0.8;
	}

	save += power * damagePerCell;

	if (!save)
		return 0;

	if (save > damage)
		save = damage;

	SpawnDamage (pa_te_type, point, normal, save);
	ent->powerarmor_time = level.time + 0.2;

	power_used = ceil((float) save / damagePerCell) + 1;

	if (cl_ent)
	{
		if (power_used > cl_ent->client->pers.inventory[index])
			power_used = cl_ent->client->pers.inventory[index];

		cl_ent->client->pers.inventory[index] -= power_used;
	}
	else
	{
		if (power_used > ent->monsterinfo.power_armor_power)
			power_used = ent->monsterinfo.power_armor_power;

		ent->monsterinfo.power_armor_power -= power_used;
	}

	return save;
}

static int CheckArmor(edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags)
{
	gclient_t	*client;
	int			save;
	int			index;
	int			max_armor;			//GHz: Max armor can absorb
	float		damage_per_armor;	//GHz: Efficiency
	float		armor_protection;
//	int talentLevel;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;
	// player-monsters dont use normal armor
	if (ent->mtype)
		return 0;
	// flag carrier can't use armor in CTF
	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;
	if (dflags & DAMAGE_NO_ABILITIES)
		return 0;

	index = ArmorIndex (ent);
	if (!index)
		return 0;

	if (dflags & DAMAGE_ENERGY)	armor_protection = 0.6;//was 0.7;
	else						armor_protection = 0.8;//was 0.9;

	//Talent: Alloyed Steel
	/*
	talentLevel = getTalentLevel(ent, TALENT_IMP_EFF_RESIST);
	if(talentLevel > 0)		armor_protection += 0.05 * talentLevel;	//+5% per upgrade.
	*/
	if (armor_protection > 0.8)
		armor_protection = 0.8;
	
	save = damage * armor_protection;
	if(!ent->myskills.abilities[ARMOR_UPGRADE].disable)
	{
		//talentLevel = getTalentLevel(ent, TALENT_IMP_EFF_POWER);
		damage_per_armor = 1 + 0.3*ent->myskills.abilities[ARMOR_UPGRADE].current_level;
		
		//Talent: Armor Mastery
		/*
		if(talentLevel > 0)
            damage_per_armor *= 1 + 0.05 * talentLevel;	//5% more per upgrade.
		*/
	}
	else damage_per_armor = 1;
	max_armor = client->pers.inventory[body_armor_index]*damage_per_armor;

	if (save >= max_armor)
		save = max_armor;

	if (!save)
		return 0;

	//gi.dprintf("raw damage = %d save = %d max_armor = %d\n", damage, save, max_armor);

	client->pers.inventory[index] -= save / damage_per_armor;

	SpawnDamage (te_sparks, point, normal, save);

	return save;
}

qboolean CanUseVampire (edict_t *targ, edict_t *attacker, int dflags, int mod)
{
	int dtype = G_DamageType(mod, dflags);

	if (!attacker->client)
		return false;

	// can't vamp from yourself
	if (targ == attacker)
		return false;

	// flag carrier can't use abilities in CTF mode
	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(attacker))
		return false;

	// is vampire upgraded?
	if (attacker->myskills.abilities[VAMPIRE].current_level < 1)
	{
		// are we a brain or mutant?
		if ((attacker->mtype != MORPH_BRAIN) && (attacker->mtype != MORPH_MUTANT))
			return false;
		// is morph mastery upgraded?
		if (attacker->myskills.abilities[MORPH_MASTERY].current_level < 1)
			return false;
	}

	// curse disables vampire
	if (que_findtype(attacker->curses, NULL, CURSE))
		return false;

	return (attacker->client && (attacker->health > 0) && (dtype & D_PHYSICAL)
		&& (targ->mtype != M_FORCEWALL) && (targ->mtype != MOD_LASER_DEFENSE));
}

void DeflectHitscan (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, float damage, int knockback, int dflags, int mod)
{		
	que_t	*slot = NULL;	
	float	modifier, chance;
	trace_t tr;
	vec3_t	newDir, end;

	// no damage
	if (damage < 1)
		return;

	// target does not have a deflect aura
	if (!(slot = que_findtype(targ->curses, NULL, DEFLECT)))
		return;

	// rough check that this is indeed a hitscan attack (otherwise inflictor and attacker would be different)
	if (inflictor && inflictor != attacker)
		return;

	modifier = DEFLECT_HITSCAN_ABSORB_BASE+DEFLECT_HITSCAN_ABSORB_ADDON*slot->ent->owner->myskills.abilities[DEFLECT].current_level;
	chance = DEFLECT_INITIAL_HITSCAN_CHANCE+DEFLECT_ADDON_HITSCAN_CHANCE*slot->ent->owner->myskills.abilities[DEFLECT].current_level;

	// cap chance
	if (chance > DEFLECT_MAX_HITSCAN_CHANCE)
		chance = DEFLECT_MAX_HITSCAN_CHANCE;

	if (chance > random())
	{
		// cap absorbtion
		if (modifier > DEFLECT_HITSCAN_ABSORB_MAX)
			modifier = DEFLECT_HITSCAN_ABSORB_MAX;

		// try to deflect damage in the opposite direction
		//VectorNegate(dir, newDir);
		//VectorMA(point, 8192, newDir, end);
		G_EntMidPoint(attacker, end);
		VectorSubtract(end, point, newDir);
		tr = gi.trace(point, NULL, NULL, end, targ, MASK_SHOT);

		// try to hurt them if they are not blessed with deflect (avoid infinite loop)
		if (tr.ent && tr.ent->takedamage && (que_findtype(tr.ent->curses, NULL, DEFLECT) == NULL))
			T_Damage(tr.ent, targ, targ, newDir, tr.endpos, tr.plane.normal, damage, knockback, dflags, mod);
				
		// deflect may absorb some or all damage
		damage *= 1.0 - modifier;
	}		
}

void G_ApplyVampire(edict_t *attacker, float take)
{
	float temp;
	float steal;
	int	delta, armorVampBase;
	int	*armor = &attacker->client->pers.inventory[body_armor_index];
	int	max_health = attacker->max_health;

	temp = 0.090*attacker->myskills.abilities[VAMPIRE].current_level;

	// brains and mutants with morph mastery use vamp
	if (attacker->mtype)
		temp = 0.25;

	steal = (int)floor(0.5 + take*temp); // steal health
	armorVampBase = steal; // save this value for armor vamp

	delta = max_health - attacker->health;
	// don't give more health than we need
	if (steal > delta)
		steal = delta;
	if (delta < 0) // players sometimes go over maximum health
		steal = 0;

	if (attacker->mtype) // morphs don't use healthcache
		attacker->health += steal;
	else
		attacker->health_cache += steal;

	//Talent: Armor Vampire
	if (*armor < MAX_ARMOR(attacker) && getTalentLevel(attacker, TALENT_ARMOR_VAMP) > 0)
	{
		//16.6% per point of health stolen gives armor as a bonus.
		float mult = 0.1666 * getTalentLevel(attacker, TALENT_ARMOR_VAMP);
		attacker->armor_cache += (int)(mult * (float)armorVampBase);
	}
}

int G_AutoTBall(edict_t *targ, float take)
{
	//3.0 try to auto-tball away when hit
	if ((targ->client)															//target must be a player
		&& G_EntIsAlive(targ)													//target must be alive
		&& (targ->health - take < (0.25 * MAX_HEALTH(targ)))					//target must end up with < 25% hp
		&& !(targ->v_flags & SFLG_AUTO_TBALLED)									//target must not have auto-tballed this spawn
		&& !HasFlag(targ))	//target must not have the flag
	{
		qboolean found = false;
		int i;
		//try to find an auto-tball
		for (i = 3; i < MAX_VRXITEMS; ++i)
		{
			if (targ->myskills.items[i].itemtype & ITEM_AUTO_TBALL)
			{
				found = true;
				break;
			}
		}
		if (found && (GetRandom(0, 100) > 75))	//75% chance of not working
		{
			//teleport them!
			Teleport_them(targ);

			//notify everyone
			gi.bprintf(PRINT_MEDIUM, "%s was teleported away by an Auto-Tball!\n", targ->myskills.player_name);

			targ->v_flags ^= SFLG_AUTO_TBALLED;
			//consume an item
			if (!(targ->myskills.items[i].itemtype & ITEM_UNIQUE))
				targ->myskills.items[i].quantity -= 1;
			if (targ->myskills.items[i].quantity == 0)
				V_ItemClear(&targ->myskills.items[i]);
			return 1;
		}
	}
	return 0;
	//end 3.0 auto-tball
}

int TotalPlayersInGame(void);
void AddDmgList (edict_t *self, edict_t *other, int damage);
void tech_checkrespawn (edict_t *ent);
qboolean curse_add(edict_t *target, edict_t *caster, int type, int curse_level, float duration);//4.4
void CurseMessage (edict_t *caster, edict_t *target, int type, float duration, qboolean isCurse);//4.4
int T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, 
			   vec3_t dir, vec3_t point, vec3_t normal, float damage, int knockback, int dflags, int mod)
{
	gclient_t	*client;
	float		take;
	float		save;
	int			asave;
	int			psave;
	int			te_sparks;
	int			talentLevel;
	//int			thorns_dmg;//GHz
	int			steal;//GHz
	float		before_sub;//GHz
	int			dtype = G_DamageType(mod, dflags);
	qboolean	plagued=false;
	float		temp=0;//K03
	edict_t		*player = G_GetClient(attacker);
	vec3_t zvec={0,0,0};//GHz
	float		startDamage = damage; //doomie
	upgrade_t	*ability;//4.2 for fury
	qboolean	target_has_pilot = PM_MonsterHasPilot(targ);
	qboolean	attacker_has_pilot = PM_MonsterHasPilot(attacker);
	que_t		*slot=NULL;

	//gi.dprintf("T_damage\n");
	// 3.7 respawn flag if it's being crushed
	if (mod == MOD_CRUSH || mod == MOD_TRIGGER_HURT) // vrxchile 2.5: or we fall into nothingness
	{
		//gi.dprintf("flag is being crushed\n");
		CTF_CheckFlag(targ);
		tech_checkrespawn(targ);
	}

	if (!targ->takedamage)
		return 0;
	if (!targ->inuse)
		return 0;

//	gi.dprintf("original damage: %d target: %s (%.1f)\n", damage, targ->classname, level.time);
	before_sub = damage = G_AddDamage(targ, inflictor, attacker, point, damage, dflags, mod);

	//4.1 some abilities that increase damage are seperate from all the others.
	if(player && (attacker_has_pilot || attacker->client) && (dtype & D_PHYSICAL))
	{
		edict_t *totem;

		//Calculate the damage bonus the player has already received.
		double x = (double)(damage - startDamage) / startDamage;

        //4.1 earth totem strength bonus.
		totem = NextNearestTotem(player, TOTEM_EARTH, NULL, true);
		if(totem && x < totem->monsterinfo.level * EARTHTOTEM_DAMAGE_MULT)
			damage = startDamage * (1.0 + EARTHTOTEM_DAMAGE_MULT * totem->monsterinfo.level);
	}

	// try to deflect an attack first before all other resistances are checked
	DeflectHitscan(targ, inflictor, attacker, dir, point, normal, damage, knockback, dflags, mod);

	damage = G_SubDamage(targ, inflictor, attacker, point, damage, dflags, mod);

	//4.1 some abilities that reduce damage are seperate from all the others.
	if(target_has_pilot || targ->client)
	{
		edict_t *totem;
		int resistLevel = 0;

		//Calculate the amount of resist the player has already received.
		double x = (double)(startDamage - damage) / startDamage;
		
		//Talent: Stone.
		totem = NextNearestTotem(G_GetClient(targ), TOTEM_EARTH, NULL, true);
		if(totem && totem->activator)
		{
			int resistLevel = getTalentLevel(totem->activator, TALENT_STONE);
			if(x < resistLevel * EARTHTOTEM_RESIST_MULT)
				damage = startDamage * (1.0 - EARTHTOTEM_RESIST_MULT * resistLevel);
		}
	}

	// lower resist curse
	if ((slot = que_findtype(targ->curses, NULL, LOWER_RESIST)) != NULL)
	{
		float lowerResistFactor = LOWER_RESIST_INITIAL_FACTOR + LOWER_RESIST_ADDON_FACTOR * slot->ent->owner->myskills.abilities[LOWER_RESIST].current_level;
		float resistFactor = (float)(startDamage - damage) / startDamage;
		float result = resistFactor - lowerResistFactor;

	//	gi.dprintf("lower resist = %.1f resist = %.1f result = %.1f startdamage = %d", lowerResistFactor, resistFactor, result, damage);
		if (damage > 0)
			damage = startDamage * (1.0 - result);
	//	gi.dprintf(" enddamage = %d\n", damage);

	}

	G_ModifyDamage(targ, inflictor, attacker, point, damage, dflags, mod);
	//damage = VortexModifyDamage(targ, inflictor, attacker, point, damage, dflags, mod);
//	gi.dprintf("%d damage\n", damage);
	
	if (targ->flags & FL_CHATPROTECT || trading->value)
		knockback = 0;

	//Talent: Mag Boots
	if (targ->client && (talentLevel = getTalentLevel(targ, TALENT_MAG_BOOTS)) > 0)
		knockback *= 1.0 - 0.16 * talentLevel;

	// assault cannon users receive no knockback
	/*
	if(targ && targ->client && targ->client->pers.weapon && (targ->client->weapon_mode)
		&& (targ->client->pers.weapon->weaponthink == Weapon_Chaingun))
		knockback = 0;
	*/
	
	// teammates can't knock your summonables around
	if (OnSameTeam(targ, attacker) && (attacker != targ) && !(targ->activator && (targ->activator == attacker)) 
		&& !(targ->creator && (targ->creator == attacker)))
		knockback = 0;

	// CE does not kick other corpses
	if ((targ->deadflag == DEAD_DEAD) && (inflictor->deadflag == DEAD_DEAD)
		&& (targ != inflictor))
		knockback = 0;

	// player bosses don't receive knockback
	if (targ->activator && G_EntIsAlive(targ) && IsABoss(targ))
	{
		if (attacker == targ)
			return 0; // boss can't hurt himself
		knockback = 0;
	}

	// can't hurt world monsters that are spawning
	//if (targ->activator && !targ->activator->client && (targ->svflags & SVF_MONSTER) 
	//	&& (targ->deadflag != DEAD_DEAD) && (targ->nextthink-level.time > 2*FRAMETIME))
	//{
	//	damage = 0;
	//	knockback = 0;
	//}
	
	// proxy owner can't be hurt or receive knockback
	if ((mod == MOD_PROXY) && (attacker == targ))
	{
		damage = 0;
		knockback = 0;
	}
	
	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if ((targ != attacker) && ((deathmatch->value && ((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->value))
	{
		if (OnSameTeam (targ, attacker))
		{
			if ((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
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
//GHz START
	if (damage > 0)
	{
		edict_t *cl;

		if (target_has_pilot)
			cl = targ->activator;
		else
			cl = targ;

		// resistance tech sound
		if (cl->client && cl->client->pers.inventory[resistance_index] 
			&& (level.time > cl->client->ctf_techsndtime))
		{
			gi.sound(cl, CHAN_ITEM, gi.soundindex("ctf/tech1.wav"), 1, ATTN_NORM, 0);
			cl->client->ctf_techsndtime = level.time + 0.9;
		}

		// if the player has a summonable, then treat its damage as if
		// the player did the damage himself
		//player = G_GetClient(attacker);
		if (player)
		{
			// keep a counter for rapid-fire weapons so we have a more
			// accurate reading of their damage over time
			if (level.time-player->lastdmg <= 0.2 && player->dmg_counter <= 32767)				
				player->dmg_counter += damage;
			else
				player->dmg_counter = damage;
			
			//if (!(player->myskills.abilities[ID].disable) && (player->myskills.abilities[ID].current_level))
				player->client->ps.stats[STAT_ID_DAMAGE] = player->dmg_counter;
			
			player->lastdmg = level.time;
			player->client->idle_frames = 0; // player is no longer idle! (uncloak em!)
		}
	
		attacker->monsterinfo.idle_frames = 0;
		
		// de-cloak obstacle
		if (attacker->mtype == M_OBSTACLE)
			attacker->svflags &= ~SVF_NOCLIENT;

		attacker->lastdmg = level.time; // last time damage was dealt
	}
//GHz END

// figure momentum add
	if (!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if (knockback && (targ->movetype != MOVETYPE_NONE) && (targ->movetype != MOVETYPE_BOUNCE) 
			&& (targ->movetype != MOVETYPE_PUSH) && (targ->movetype != MOVETYPE_STOP) 
			&& !que_typeexists(targ->curses, CURSE_FROZEN))
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

	// check for invincibility
	if ((client && client->invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->pain_debounce_time < level.time)
		{
			gi.sound(targ, CHAN_ITEM, gi.soundindex("items/protect3.wav"), 1, ATTN_NORM, 0);
			targ->pain_debounce_time = level.time + 2;
		}
		take = 0;
		save = damage;
	}

	//dtype = G_DamageType(mod, dflags);
	if ((dtype & D_PHYSICAL) && HitTheWeapon(targ, attacker, point, take, dflags))
	{
		TossClientWeapon(targ); // Make them toss weapon..
		take *= 0.25;
	}

	// check for godmode
	if ( (targ->flags & FL_GODMODE) && !(dflags & DAMAGE_NO_PROTECTION) )
	{
		// only admins can use god mode!
		if (targ->client && targ->myskills.administrator)	
		{
			take = 0;
			save = damage;
			if ((level.time > pregame_time->value) && (G_ValidTarget(targ, attacker, false)))
				SpawnDamage (TE_BLOOD, point, normal, save);
		}
	}

	// entity has been hurt
	if (attacker && (attacker != world) && (take > 0))// && (mod != MOD_BURN) && (mod != MOD_PLAGUE))
	{
		targ->lasthurt = level.time;
		if (target_has_pilot)
		{
			targ->activator->client->idle_frames = 0;
			targ->activator->lasthurt = level.time;
		}

		if (targ->client)
			targ->client->idle_frames = 0;
		else
			targ->monsterinfo.idle_frames = 0;

		// de-cloak obstacle
		if (targ->mtype == M_OBSTACLE)
			targ->svflags &= ~SVF_NOCLIENT;

		
		//Talent: Dim Vision
		if ((dtype & D_PHYSICAL) && targ->client && G_EntIsAlive(targ) && ((talentLevel = getTalentLevel(targ, TALENT_DIM_VISION)) > 0)
			&& (level.framenum > targ->dim_vision_delay) && (targ != attacker)) // 10% chance per 1 second
			// az- fix autocurse
		{
			int curse_level;
			
			temp = 0.04 * talentLevel;
			if (temp > random())
			{
				curse_level = targ->myskills.abilities[CURSE].current_level;

				if (curse_level < 1)
					curse_level = 1;

				// add the curse
				curse_add(attacker, targ, CURSE, curse_level, curse_level/2);
				CurseMessage(targ, attacker, CURSE, curse_level, true);
			}

			// modify delay
			targ->dim_vision_delay = level.framenum + 10; // roll again in 1 second
		}



		////////////////////////TALENT : FAST MOVE (RUN!) ALIEN /////////////////////////////////////

	//	if ((dtype & D_PHYSICAL) && targ->client && G_EntIsAlive(targ) && ((talentLevel = getTalentLevel(targ, TALENT_FASTMOVE)) > 0)
		//	&& (level.framenum > targ->dim_vision_delay) && (targ != attacker)) // 10% chance per 1 second
			// az- fix autocurse
//		{
	//		int curse_level;

		//	temp = 0.04 * talentLevel;
			//if (temp > random())
			//{
				//curse_level = targ->myskills.abilities[CURSE].current_level;

				//if (curse_level < 1)
					//curse_level = 1;

				// add the curse
				//curse_add(attacker, targ, CURSE, curse_level, curse_level / 2);
				//CurseMessage(targ, attacker, CURSE, curse_level, true);
			//}

			// modify delay
			//targ->dim_vision_delay = level.framenum + 10; // roll again in 1 second
		//}
		//int duration = 3;
		//attacker->fury_time = level.time + duration;
		//attacker->myskills.abilities[FURY].delay = attacker->fury_time + 1.0;



		/////////////////////////////////////////////////////////////////////////////////////////////
		//4.5 monster bonus flag ghostly chills targets
		if (G_EntIsAlive(targ) && level.framenum > targ->dim_vision_delay)
		{
			if (attacker->monsterinfo.bonus_flags & BF_GHOSTLY)
			{
				if (random() <= 0.2)
				{
					// freeze em
					targ->chill_level = 10;
					targ->chill_time = level.time + 10.0;

					if (random() > 0.5)
						gi.sound(targ, CHAN_ITEM, gi.soundindex("spells/blue1.wav"), 1, ATTN_NORM, 0);
					else
						gi.sound(targ, CHAN_ITEM, gi.soundindex("spells/blue3.wav"), 1, ATTN_NORM, 0);
					
					if (targ->client)
						safe_cprintf(targ, PRINT_HIGH, "You have been chilled for 10 seconds\n");
				}

				targ->dim_vision_delay = level.framenum + 10;
			}

			if (attacker->monsterinfo.bonus_flags & BF_STYGIAN)
			{
				if (random() <= 0.2)
				{
					// add the curse
					curse_add(targ, attacker, AMP_DAMAGE, 10, 10.0);
					CurseMessage(attacker, targ, AMP_DAMAGE, 10.0, true);
				}

				targ->dim_vision_delay = level.framenum + 10;
			}
		}
	}

	if (take > 0)
		psave = CheckPowerArmor (targ, point, normal, before_sub, dflags);
	else
		psave = CheckPowerArmor (targ, point, normal, 0, dflags);
	take -= psave;
	// need this since power armor can take more damage than normal
	if (take < 0)
		take = 0;
	asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	take -= asave;
	//treat cheat/powerup savings the same as armor
	asave += save;

// do the damage
	if (take)
	{
		if (G_AutoTBall(targ, take))
			return 0;

		if ((targ->svflags & SVF_MONSTER) || (client))
			SpawnDamage (TE_BLOOD, point, normal, take);
		else
			SpawnDamage (te_sparks, point, normal, take);

		targ->health -= take;

		//4.1 Darkness totem gives players a seperate vampire effect.
		if(player && (attacker_has_pilot || attacker->client))
		{
			edict_t *totem = NextNearestTotem(player, TOTEM_DARKNESS, NULL, true);
			int maxHP = player->max_health;//MAX_HEALTH(player);

			if(totem != NULL)
			{
				//Talent: Shadow. Players can go beyond their "normal" max health.
				maxHP *= 1.0 + DARKNESSTOTEM_MAX_MULT * getTalentLevel(totem->activator, TALENT_SHADOW);

				if(player->health < maxHP)
				{
					player->health += totem->monsterinfo.level * take * DARKNESSTOTEM_VAMP_MULT;

					if (player->health > maxHP)
						player->health = maxHP;

					if (attacker_has_pilot)//4.2 sync monster and player health
						player->owner->health = player->health;
				}
			}
		}
	
		//4.2 give hellspawn vampire ability (50% = 150hp/sec assuming 300dmg/sec)

		if (attacker->monsterinfo.bonus_flags & BF_STYGIAN)
		{
			attacker->health += take;
			if (attacker->health > 2*attacker->max_health)
				attacker->health = 2*attacker->max_health;
		}

		if (attacker->mtype == M_SKULL)
		{
			attacker->health += 0.5*take;
			if (attacker->health > 2*attacker->max_health)
				attacker->health = 2*attacker->max_health;
		}

		// spikeball has vampire ability
		if (attacker->mtype == M_SPIKEBALL)
		{
			attacker->health += take;
			if (attacker->health > attacker->max_health)
				attacker->health = attacker->max_health;
		}

		// vampire effect
		if (CanUseVampire(targ, attacker, dflags, mod))
			G_ApplyVampire(attacker, take);

		//4.1 Players with fury might get their ability triggered
		ability = &attacker->myskills.abilities[FURY];
		if (attacker != targ && dtype & D_PHYSICAL && !(dtype & D_MAGICAL) && ability->current_level > 0 && ability->delay < level.time)
			G_ApplyFury(attacker);
		
		if (targ->health <= 0)
		{
			// if the attacker is a player, add them to this entity's damage list
			if (player)
			{
				if (target_has_pilot)
					AddDmgList(targ->owner, player, take+targ->health+asave+psave);
				else
					AddDmgList(targ, player, take+targ->health+asave+psave);
			}

			if ((targ->svflags & SVF_MONSTER) || (client))
				targ->flags |= FL_NO_KNOCKBACK;
			Killed (targ, inflictor, attacker, take, point);
			return take;
		}
	}
	
	// if the attacker is a player, add them to this entity's damage list
	if (player)
	{
		if (target_has_pilot)
			AddDmgList(targ->owner, player, damage);
		else
			AddDmgList(targ, player, damage);
	}

	if (client)
	{
		if (!(targ->flags & FL_GODMODE) && (take))
			targ->pain (targ, attacker, knockback, take);
	}
	else if (take)
	{
		if (targ->pain)
			targ->pain (targ, attacker, knockback, take);

		//3.9 add damage effects for player-monster
		if (targ->mtype == P_TANK)
		{
			if (targ->activator && targ->activator->inuse)
			{
				targ->activator->client->damage_parmor += psave;
				targ->activator->client->damage_armor += asave;
				targ->activator->client->damage_blood += take;
				targ->activator->client->damage_knockback += knockback;
				VectorCopy (point, targ->activator->client->damage_from);
			}
		}

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

	return take;
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
		if (!visible1(inflictor, ent))
			continue;

		//gi.dprintf("damage = %.0f radius = %.0f range = %.0f\n", damage, radius, entdist(inflictor, ent));

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		//K03 Begin
		points = damage - damage * 0.5 *((VectorLength(v))/radius);

	//	gi.dprintf("points = %.1f\n", points);
		//points = damage - 0.5 * VectorLength (v);
		//K03 End
		// points = damage * (1 - (VectorLength(v) / radius));
		// points = damage * (1 - (VectorLength(v) / radius)^2);
		if (ent == attacker)
		{
			// reduce self-inflicted damage
			if (mod == MOD_EXPLODINGARMOR)
				points *= 0.25;
			else
				points *= 0.5;
		}

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				int knockback = (int) points;

				//4.2 limit knockback
				if (knockback > MAX_KNOCKBACK)
					knockback = MAX_KNOCKBACK;

				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, knockback, DAMAGE_RADIUS, mod);
			}
		}
	}
}

void T_RadiusDamage_Players (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod) //only affects players
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
		if (!visible1(inflictor, ent))
			continue;
		if (!ent->client)
			continue;

		//gi.dprintf("damage = %.0f radius = %.0f range = %.0f\n", damage, radius, entdist(inflictor, ent));

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		//K03 Begin
		points = damage - damage * 0.5 *((VectorLength(v))/radius);

	//	gi.dprintf("points = %.1f\n", points);
		//points = damage - 0.5 * VectorLength (v);
		//K03 End
		// points = damage * (1 - (VectorLength(v) / radius));
		// points = damage * (1 - (VectorLength(v) / radius)^2);
		if (ent == attacker)
		{
			// reduce self-inflicted damage
			if (mod == MOD_EXPLODINGARMOR)
				points *= 0.25;
			else
				points *= 0.5;
		}

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				int knockback = (int) points;

				//4.2 limit knockback
				if (knockback > MAX_KNOCKBACK)
					knockback = MAX_KNOCKBACK;

				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, knockback, DAMAGE_RADIUS, mod);
			}
		}
	}
}

void T_RadiusDamage_Nonplayers (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod) //only affects players
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
		if (!visible1(inflictor, ent))
			continue;
		if (ent->client)
			continue;

		//gi.dprintf("damage = %.0f radius = %.0f range = %.0f\n", damage, radius, entdist(inflictor, ent));

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (inflictor->s.origin, v, v);
		//K03 Begin
		points = damage - damage * 0.5 *((VectorLength(v))/radius);

	//	gi.dprintf("points = %.1f\n", points);
		//points = damage - 0.5 * VectorLength (v);
		//K03 End
		// points = damage * (1 - (VectorLength(v) / radius));
		// points = damage * (1 - (VectorLength(v) / radius)^2);
		if (ent == attacker)
		{
			// reduce self-inflicted damage
			if (mod == MOD_EXPLODINGARMOR)
				points *= 0.25;
			else
				points *= 0.5;
		}

		if (points > 0)
		{
			if (CanDamage (ent, inflictor))
			{
				int knockback = (int) points;

				//4.2 limit knockback
				if (knockback > MAX_KNOCKBACK)
					knockback = MAX_KNOCKBACK;

				VectorSubtract (ent->s.origin, inflictor->s.origin, dir);
				T_Damage (ent, inflictor, attacker, dir, inflictor->s.origin, vec3_origin, (int)points, knockback, DAMAGE_RADIUS, mod);
			}
		}
	}
}
