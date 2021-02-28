#include "g_local.h"
#include "damage.h"

int G_DamageType (int mod, int dflags)
{
	switch (mod)
	{
	// world damage
	case MOD_FALLING:
	case MOD_WATER:
	case MOD_LAVA:
	case MOD_SLIME:
		return D_WORLD;
	/*
	Monster damage is classed as MOD_UNKNOWN
	So lets make sure it works with thorns
	*/
	// all morphed player attacks should go here
	case MOD_UNKNOWN:
	case MOD_HIT:
	case MOD_BRAINTENTACLE:
	case MOD_TENTACLE:
	case MOD_MUTANT:
	case MOD_BERSERK_CRUSH:
	case MOD_BERSERK_SLASH:
	case MOD_BERSERK_PUNCH:
	case MOD_CACODEMON_FIREBALL:
	case MOD_PARASITE:
	case MOD_TANK_PUNCH:
	case MOD_TANK_BULLET:
		return (D_PHYSICAL | D_MAGICAL);
	// magic attacks
	case MOD_SENTRY:
		return (D_MAGICAL | D_BULLET);
	case MOD_BOMBS:
	case MOD_CORPSEEXPLODE:
	case MOD_SENTRY_ROCKET:
	case MOD_PROXY:
	case MOD_NAPALM:
	case MOD_METEOR:
	case MOD_FIREBALL:
	case MOD_MIRV:
		return (D_MAGICAL | D_EXPLOSIVE);
	//case MOD_HOLYSHOCK:
	case MOD_LIGHTNING:
	case MOD_MAGICBOLT:
	case MOD_SKULL:
	case MOD_BEAM:
	case MOD_PLASMABOLT:
	case MOD_LIGHTNING_STORM:
		return (D_MAGICAL | D_ENERGY);
	case MOD_BURN:
	case MOD_CORPSEEATER:
	case MOD_PLAGUE:
	case MOD_CRIPPLE:
	case MOD_NOVA:
	case MOD_MINDABSORB:
	case MOD_FIRETOTEM:
	case MOD_WATERTOTEM:
	case MOD_SPIKEGRENADE:
	case MOD_CALTROPS:
	
	case MOD_SPIKE:
	case MOD_GAS:
	case MOD_OBSTACLE:
	case MOD_SPORE:
	case MOD_ACID:
		return D_MAGICAL;
	// explosive attacks
	case MOD_ROCKET:
	case MOD_R_SPLASH:
	case MOD_GRENADE:
	case MOD_G_SPLASH:
	case MOD_HANDGRENADE:
	case MOD_HG_SPLASH:
	case MOD_HELD_GRENADE:
		return (D_EXPLOSIVE | D_PHYSICAL);
	case MOD_EXPLODINGARMOR:
	case MOD_EXPLOSIVE:
	case MOD_BARREL:
	case MOD_BOMB:
	case MOD_SUPPLYSTATION:
		return D_EXPLOSIVE;
	// energy attacks
	case MOD_BFG_BLAST:
	case MOD_BFG_LASER:
	case MOD_BFG_EFFECT:
	case MOD_HYPERBLASTER:
	case MOD_BLASTER:
	case MOD_SWORD:
		return (D_ENERGY | D_PHYSICAL);
	case MOD_LASER_DEFENSE:
		return D_ENERGY; 
	// shell weapons
	case MOD_SSHOTGUN:
	case MOD_SHOTGUN:
		return (D_SHELL | D_PHYSICAL);
	// pierce weapons
	case MOD_RAILGUN:
	case MOD_SNIPER:
	case MOD_CANNON:
		return (D_PIERCING | D_PHYSICAL);
	// bullet weapons
	case MOD_MACHINEGUN:
	case MOD_CHAINGUN:
		return (D_BULLET | D_PHYSICAL);
	}

	// try these next
	if (dflags & DAMAGE_BULLET)
		return (D_BULLET | D_PHYSICAL);
	if (dflags & DAMAGE_ENERGY)
		return (D_ENERGY | D_PHYSICAL);
	if (dflags & DAMAGE_PIERCING)
		return (D_PIERCING | D_PHYSICAL);
	if (dflags & DAMAGE_RADIUS)
		return (D_EXPLOSIVE | D_PHYSICAL);

	return 0;
}	

qboolean IsPhysicalDamage (int dtype, int mod)
{
	return ((dtype & D_PHYSICAL) && (mod != MOD_LASER_DEFENSE) && !(dtype & D_WORLD) && !(dtype & D_MAGICAL));
}

qboolean IsMorphedPlayer (edict_t *ent)
{
	return (PM_MonsterHasPilot(ent) || ent->mtype == P_TANK || ent->mtype == MORPH_MUTANT || ent->mtype == MORPH_CACODEMON 
		|| ent->mtype == MORPH_BRAIN || ent->mtype == MORPH_FLYER || ent->mtype == MORPH_MEDIC 
		|| ent->mtype == MORPH_BERSERK);
}

float getPackModifier (edict_t *ent)
{
	//Talent: Pack Animal
	int		talentLevel;
	edict_t *e = NULL;

	if (ent->client)
		talentLevel=getTalentLevel(ent, TALENT_PACK_ANIMAL);
	else if (ent->owner && ent->owner->inuse && ent->owner->client)
		talentLevel=getTalentLevel(ent->owner, TALENT_PACK_ANIMAL);

	// talent isn't upgraded or we are not morphed
	if (talentLevel < 1 || !IsMorphedPlayer(ent))
		return 1.0;

	// find nearby pack animals
	while ((e = findradius (e, ent->s.origin, 1024)) != NULL)
	{
		if (e == ent)
			continue;
		// must be alive
		if (!G_EntIsAlive(e))
			continue;
		// must not be in chat protect mode
		if (e->flags & FL_CHATPROTECT)
			continue;
		// must be on our team
		if (!OnSameTeam(ent, e))
			continue;
		// must be morphed
		if (!IsMorphedPlayer(e))
			continue;
		return 1.0 + 0.1 * talentLevel; // +10% per level
	}

	return 1.0;
}

float G_AddDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, 
				   vec3_t point, float damage, int dflags, int mod)
{
	int		dtype;
	float	temp;
	que_t	*slot = NULL;
	qboolean physicalDamage;
	int talentLevel;

	dtype = G_DamageType(mod, dflags);

	// cripple does not get any damage bonuses, as it is already very powerful (% health)
	if (mod == MOD_CRIPPLE)
		return damage;

	// spirits shoot a blaster, but it should be (D_MAGICAL | D_ENERGY)
	if (attacker->mtype == M_YANGSPIRIT)	
		dtype = D_MAGICAL | D_ENERGY;

	// monster/sentry/etc. and morphed players should be considered
	// magical attacks if they are not already
	if ((attacker->mtype || attacker->activator || attacker->creator) && !(dtype & D_MAGICAL))
		dtype |= D_MAGICAL;

	physicalDamage = IsPhysicalDamage(dtype, mod);

	// monsters get invincibility and quad in invasion mode
	if (!attacker->client && (attacker->monsterinfo.inv_framenum > level.framenum))
		damage *= 4; 

	// 4.5 monster bonus flags
	if (attacker->monsterinfo.bonus_flags & BF_CHAMPION)
		damage *= 2;
	if (attacker->monsterinfo.bonus_flags & BF_BERSERKER)
		damage *= 2;

	// hellspawn gains extra damage against non-clients
	if ((attacker->mtype == M_SKULL) && (targ->mtype != M_SKULL) && !targ->client)
	{
		if (pvm->value || invasion->value)
			damage *= 4;
		else
			damage *= 1.5;
	}

	// proxy, caltrops and medic packs are 3 more effective against non-players
	if (((mod == MOD_CALTROPS) || (mod == MOD_PROXY) || (mod == MOD_FMEDICPACK)) && !targ->client && targ->mtype)
		damage *= 3;

	if (ctf->value)
	{
		int		delta = abs(red_flag_caps-blue_flag_caps);
		edict_t *cl_targ = G_GetClient(targ);

		if ((delta > 1) && cl_targ) // need at least 2 caps
		{
			// team with more flag captures takes more damage
			if ((cl_targ->teamnum == RED_TEAM) && (red_flag_caps > blue_flag_caps))
				temp = 0.15*delta;
			else if ((cl_targ->teamnum == BLUE_TEAM) && (blue_flag_caps > red_flag_caps))
				temp = 0.15*delta;
			else
				temp = 0;

			// cap maximum damage to 2x
			if (temp > 1.0)
				temp = 1.0;
			
			if (temp > 0)
				damage *= 1 + temp;
		}
	}


	//Start Talent: Monster Mastery
	if (getTalentSlot(attacker, TALENT_MONSTER_MASTERY) != -1)
	{
		int level = getTalentLevel(attacker, TALENT_MONSTER_MASTERY);
		float temp;

		if (pvm->value || invasion->value)
		{
			temp = level * 0.06; damage *= 1.0 + temp;
		}
			else
			{
			temp = 0;
			}
	}
	// END Talent: Monster Mastery


	if (dtype & D_PHYSICAL)
	{
		// cocoon bonus
		if (attacker->cocoon_time > level.time)
			damage *= attacker->cocoon_factor;

		// player-monster damage bonuses
		if (!attacker->client && attacker->mtype && PM_MonsterHasPilot(attacker))
		{
			// morphed players' attacks deal more damage after level 10
			int levels = attacker->owner->myskills.level-10;

			// Talent: Retaliation
			// increases damage as percentage of remaining health is reduced
			talentLevel = getTalentLevel(attacker->owner, TALENT_RETALIATION);
			if (talentLevel > 0)
			{
				temp = attacker->health / (float)attacker->max_health;
				if (temp > 1)
					temp = 1;
				damage *= 1.0 + ((0.2 * talentLevel) * (1.0 - temp));
			}
			

			// Talent: Superiority
			// increases damage/resistance of morphed players against monsters
			talentLevel = getTalentLevel(attacker->owner, TALENT_SUPERIORITY);
			if (talentLevel > 0 && targ->activator && targ->mtype != P_TANK && targ->svflags & SVF_MONSTER)
				damage *= 1.0 + 0.2*talentLevel;

			// Talent: Pack Animal
			damage *= getPackModifier(attacker);

			if (levels > 0)
				damage *= 1 + 0.05*levels;

			// strength tech effect
			if (attacker->owner && attacker->owner->inuse && attacker->owner->client
				&& attacker->owner->client->pers.inventory[strength_index])
			{
				if (attacker->owner->myskills.level <= 5)
					damage *= 2;
				else if (attacker->owner->myskills.level <= 10)
					damage *= 2;
				else
					damage *= 2;
			}
		}

		// targets cursed with "amp damage" take additional damage
		if ((slot = que_findtype(targ->curses, NULL, AMP_DAMAGE)) != NULL)
		{
			temp = AMP_DAMAGE_MULT_BASE + (slot->ent->owner->myskills.abilities[AMP_DAMAGE].current_level * AMP_DAMAGE_MULT_BONUS);
			// cap amp damage at 3x damage
			if (temp > 3)
				temp = 3;
			damage *= temp;
		}

		// weaken causes target to take more damage
		if ((slot = que_findtype(targ->curses, NULL, WEAKEN)) != NULL)
		{
			temp = WEAKEN_MULT_BASE + (slot->ent->owner->myskills.abilities[WEAKEN].current_level * WEAKEN_MULT_BONUS);
			damage *= temp;
		}

		// targets "chilled" deal less damage.
		if(attacker->chill_time > level.time)
			damage /= 1.0 + 0.0333 * (float)attacker->chill_level; //chill level 10 = 25% less damage

		// targets cursed with "weaken" deal less damage
		if ((slot = que_findtype(attacker->curses, NULL, WEAKEN)) != NULL)
			damage /= WEAKEN_MULT_BASE + (slot->ent->owner->myskills.abilities[WEAKEN].current_level * WEAKEN_MULT_BONUS);
	}

	// it becomes increasingly difficult to hold onto the flag
	// after the first minute in domination mode
	if (domination->value && DEFENSE_TEAM && (targ->teamnum == DEFENSE_TEAM) && (FLAG_FRAMES > 600))
	{
		//FIXME: teammates can swap the flag to get around this!
		temp = 1 + (float)FLAG_FRAMES/1800; // 3 minutes to max difficulty
		if (temp > 2)
			temp = 2;
		damage *= temp;
	}

	// attackers blessed deal additional damage
	if (que_findtype(attacker->curses, NULL, BLESS) != NULL)
	{
		float bonus;

		if (dtype & D_MAGICAL)
			bonus = BLESS_MAGIC_BONUS;
		else
			bonus = BLESS_BONUS;

		damage *= bonus;
	}

	// player-only damage bonuses
	if (attacker->client && (attacker != targ))
	{
		// increase physical or morphed-player damage
		if (dtype & D_PHYSICAL)
		{
			// strength tech effect
			if (attacker->client->pers.inventory[strength_index])
			{
				if (attacker->myskills.level <= 5)
					damage *= 2;
				else if (attacker->myskills.level <= 10)
					damage *= 1.5;
				else
					damage *= 1.25;
			}

			if (attacker->mtype)
			{
				// morphed players' attacks deal more damage after level 10
				int levels = attacker->myskills.level-10;
#pragma region RETALIATION TALENT
				// Talent: Retaliation
				// increases damage as percentage of remaining health is reduced
				talentLevel = getTalentLevel(attacker, TALENT_RETALIATION);
				if (talentLevel > 0)
				{
					temp = attacker->health / (float)attacker->max_health;
					if (temp > 1)
						temp = 1;
					damage *= 1.0 + ((0.2 * talentLevel) * (1.0 - temp));
				}
#pragma endregion
				// Talent: Superiority
				// increases damage/resistance of morphed players against monsters
				talentLevel = getTalentLevel(attacker, TALENT_SUPERIORITY);
				if (talentLevel > 0 && targ->activator && targ->mtype != P_TANK && targ->svflags & SVF_MONSTER)
					damage *= 1.0 + 0.2*talentLevel;

				if (levels > 0)
					damage *= 1 + 0.05*levels;

				// Talent: Pack Animal
				damage *= getPackModifier(attacker);
			}
		}

		// only physical damage is increased
		if (physicalDamage)
		{
			// handle accuracy
			if (mod != MOD_BFG_LASER)
			{
				attacker->shots_hit++;
				attacker->myskills.shots_hit++;
			}
			
			// strength effect
			if (!attacker->myskills.abilities[STRENGTH].disable)
			{
				int		talentLevel;

				temp = 1 + STRENGTH_BONUS * attacker->myskills.abilities[STRENGTH].current_level;
			
				//Talent: Endurance
				//talentLevel = getTalentLevel(attacker, TALENT_IMP_STRENGTH);
				//if (talentLevel > 0)
					//temp += IMP_STRENGTH_BONUS * talentLevel;

				//Talent: Improved Strength
				talentLevel = getTalentLevel(attacker, TALENT_IMP_STRENGTH);
				if(talentLevel > 0)		
					temp += IMP_STRENGTH_BONUS * talentLevel;
				
				// ESTAS LINEAS DISMINUYEN EL DAÑO

				//Talent: Improved Resist
			//	talentLevel = getTalentLevel(attacker, TALENT_IMP_RESIST);
				//if(talentLevel > 0)		
					//temp -= 0.1 * talentLevel;   // 

				//don't allow damage under 100%
				if (temp < 1.0) 
					temp = 1.0;

				damage *= temp;
			}
#pragma region -- COMBAT EXPERIENCE TALENT 
			//Talent: Combat Experience
			if (getTalentSlot(attacker, TALENT_COMBAT_EXP) != -1)
				damage *= 1.0 + 0.05 * getTalentLevel(attacker, TALENT_COMBAT_EXP);	//+5% per upgrade
#pragma endregion
#pragma region SOLDIER BLOOD OF ARES TALENT  
			// ******TALENT BLOOD OF ARES START {****** //
			if (getTalentSlot(attacker, TALENT_BLOOD_OF_ARES) != -1)
			{
				int level = getTalentLevel(attacker, TALENT_BLOOD_OF_ARES);
				float temp;




				// BoA is more effective in PvM
				if (pvm->value || invasion->value)
					temp = level * 0.02 *attacker->myskills.streak;
				else
					temp = level * 0.01 *attacker->myskills.streak;

				//Limit bonus to +100%
				if (temp > 1.5)	temp = 1.5;

				damage *= 1.0 + temp;
			}
			// ******TALENT BLOOD OF ARES END }****** //
#pragma endregion

		


			

			// fury ability increases damage.
			if(attacker->fury_time > level.time)
			{
				// (apple)
				// Changed to attacker instead of target's fury level!
				temp = FURY_INITIAL_FACTOR + (FURY_ADDON_FACTOR * attacker->myskills.abilities[FURY].current_level);
				if (temp > FURY_FACTOR_MAX)
					temp = FURY_FACTOR_MAX;
				damage *= temp;
			}
		}
	}

	return damage;
}

float G_SubDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, 
				   vec3_t point, float damage, int dflags, int mod)
{
	int		dtype;
	float	temp;
	que_t	*aura=NULL;
	int talentLevel;
	edict_t *dclient;
	qboolean invasion_friendlyfire = false;
	float Resistance = 1.0; // We find the highest resist value and only use THAT.

	//gi.dprintf("G_SubDamage()\n");
	//gi.dprintf("%d damage before G_SubDamage() modification\n", damage);

	dtype = G_DamageType(mod, dflags);

	dclient = G_GetClient(targ);

	if (dflags & DAMAGE_NO_PROTECTION)
		return damage;
	if (targ->deadflag == DEAD_DEAD || targ->health < 1)
		return damage; // corpses take full damage
	if (mod == MOD_TELEFRAG)
		return damage;

	if (level.time < pregame_time->value)
		return 0; // no damage in pre-game
	if (trading->value)
		return 0; // az 2.5 vrxchile: no damage in trading mode
	if (OnSameTeam(attacker, targ) && (attacker != targ))
	{
		if (invasion->value > 1 && gi.cvar("inh_friendlyfire", "0", 0)->value)
		{
			// if none of them is a client and the target is not a piloted monster
			if (!(G_GetClient(attacker) && G_GetClient(targ)))
				return 0; // then friendly fire is off.
			invasion_friendlyfire = true;
		}else
			return 0;  // can't damage teammates
	}
	if (que_typeexists(targ->curses, CURSE_FROZEN))
		return 0; // can't damage frozen entities
	if (targ->flags & FL_CHATPROTECT)
		return 0; // can't kill someone in chat-protect
	if (!targ->client && (attacker == targ) && !PM_MonsterHasPilot(targ))//4.03 added player-monster exception
		return 0; // non-clients can't hurt themselves
	if (!targ->client && (targ->monsterinfo.inv_framenum > level.framenum))
		return 0; // monsters get invincibility and quad in invasion mode
	if (targ->flags & FL_COCOONED && targ->movetype == MOVETYPE_NONE && targ->svflags & SVF_NOCLIENT)
		return 0; //4.4 don't hurt cocooned entities

	if (dflags & DAMAGE_NO_ABILITIES)
		return damage; // ignore abilities		

	// 4.5 monster bonus flags
	if (targ->monsterinfo.bonus_flags & BF_GHOSTLY && random() <= 0.5)
		return 0;

	if (invasion_friendlyfire)
	{
		Resistance = min(Resistance, 0.5);
	}

	if (hw->value) // modify damage
		damage *= hw_getdamagefactor(targ, attacker);

	//Talent: Bombardier - reduces self-inflicted grenade damage
	if (PM_GetPlayer(targ) == PM_GetPlayer(attacker) 
		&& (mod == MOD_EMP || mod == MOD_MIRV || mod == MOD_NAPALM || mod == MOD_GRENADE || mod == MOD_HG_SPLASH || mod == MOD_HANDGRENADE))
	{
		talentLevel = getTalentLevel(targ, TALENT_BOMBARDIER);
		if (mod == MOD_EMP)
			Resistance = min(Resistance, 1.0 - 0.16 * talentLevel);
		else
			Resistance = min(Resistance, 1.0 - 0.12 * talentLevel); // damage reduced to 20% at talent level 5 (explosive damage already reduced 50% in t_radiusdamage)
	}

	// hellspawn gets extra resistance against other summonables
	if ((targ->mtype == M_SKULL) && (attacker->mtype != M_SKULL) && !attacker->client && !PM_MonsterHasPilot(attacker))
	{
		if (pvm->value || invasion->value)
			Resistance = min(Resistance, 0.5);
		else
			Resistance = min(Resistance, 0.66);
	}

	// spores get extra resistance to other summonables
	if ((targ->mtype == M_SPIKEBALL) && (attacker->mtype != M_SPIKEBALL) && !attacker->client && !PM_MonsterHasPilot(attacker))
	{
		if (pvm->value || invasion->value)
			Resistance = min(Resistance, 0.5);
		else
			Resistance = min(Resistance, 0.66);
	}

	// detector is super-resistant to non-client attacks
	if ((targ->mtype == M_DETECTOR) && !attacker->client)
		Resistance = min(Resistance, 0.5);

	// resistance tech
	if (targ->client && targ->client->pers.inventory[resistance_index])
	{
		if (targ->myskills.level <= 5)
			Resistance = min(Resistance, 0.5);
		else if (targ->myskills.level <= 10)
			Resistance = min(Resistance, 0.66);
		else
			Resistance = min(Resistance, 0.8);
	}

	// cocoon bonus
	if (targ->cocoon_time > level.time)
		Resistance = min(Resistance, 1.0/targ->cocoon_factor);

	// player tank
	if (targ->mtype == P_TANK)
	{
		temp = 1.0;

		// Talent: Superiority
		// increases damage/resistance of morphed players against monsters
		talentLevel = getTalentLevel(targ, TALENT_SUPERIORITY);
		
		if (attacker->activator && attacker->mtype != P_TANK && (attacker->svflags & SVF_MONSTER) && talentLevel > 0)
			temp += 0.2 * talentLevel;

		// Talent: Pack Animal
		temp *= getPackModifier(targ);

		Resistance = min(Resistance, 1/temp);

		// resistance tech
		if (targ->owner && targ->owner->inuse && targ->owner->client 
			&& targ->owner->client->pers.inventory[resistance_index])
		{
			if (targ->owner->myskills.level <= 5)
				Resistance = min(Resistance, 0.5);
			else if (targ->owner->myskills.level <= 10)
				Resistance = min(Resistance, 0.66);
			else
				Resistance = min(Resistance, 0.8);
		}


		/// PUEDE QUE QUEDE LA CAGA
		// resistance effect (for tanks, lessened)
		if (!targ->owner->myskills.abilities[RESISTANCE].disable)
		{
			temp = 1 + 0.07 * targ->owner->myskills.abilities[RESISTANCE].current_level;

			//Talent: Improved Resist
		//	talentLevel  = getTalentLevel(targ, TALENT_IMP_RESIST);
			//if(talentLevel > 0)
				//temp += talentLevel * 0.07;

			//Talent: Improved Strength
			//talentLevel = getTalentLevel(targ, TALENT_IMP_STRENGTH);
			//if(talentLevel > 0)		
				//temp -= talentLevel * 0.07;
			
			// don't allow more than 100% damage
			if (temp < 1.0)
				temp = 1.0;

			Resistance = min(Resistance, 1/temp);
		}

		if (attacker->svflags & SVF_MONSTER) // monsters inflict only 3/4s damage to tanks
			Resistance = min(Resistance, 0.75);
	}

	// morphed players
	if (targ->client && targ->mtype)
	{
		temp = 1.0;

		// Talent: Superiority
		// increases damage/resistance of morphed players against monsters
		talentLevel = getTalentLevel(targ, TALENT_SUPERIORITY);
		if (attacker->activator && attacker->mtype != P_TANK && (attacker->svflags & SVF_MONSTER) && talentLevel > 0)
			temp += 0.2 * talentLevel;

		// Talent: Pack Animal
		temp *= getPackModifier(targ);

		Resistance = min(Resistance, 1/temp);
	}

	if (IsABoss(targ) && ((mod == MOD_BURN) || (mod == MOD_LASER_DEFENSE)))
	{
		Resistance = min(Resistance, 0.33);; // boss takes less damage from burn and lasers
	}

	// corpse explosion does not damage other corpses
	if ((targ->deadflag == DEAD_DEAD) && (inflictor->deadflag == DEAD_DEAD)
		&& (targ != inflictor))
		return 0;
	
	//4.1 air totems can reduce damage. (players only)
	if(targ->client || (targ->mtype == P_TANK))
	{
		//4.1 air totem.
		edict_t *totem = NextNearestTotem(targ, TOTEM_AIR, NULL, true);
		if(totem != NULL)
		{
			int take = 0.5*damage;
			vec3_t normal;

			//Talent: Wind. Totems have a chance to ghost an attack.
			if(GetRandom(0, 99) < 5 * getTalentLevel(totem->activator, TALENT_WIND))
				return 0;
			
			VectorSubtract(targ->s.origin, totem->s.origin, normal);

			Resistance = min(Resistance, 0.5);

			//Half of the damage goes to the totem, reisted by its skill level.
			take /= AIRTOTEM_RESIST_BASE + AIRTOTEM_RESIST_MULT * totem->monsterinfo.level;
			T_Damage(totem, inflictor, attacker, vec3_origin, targ->s.origin, normal, take, 0, dflags, mod);
		}
	}

	//3.0 Defenders blessed take less damage
	if (que_findtype(targ->curses, NULL, BLESS) != NULL)
	{
		float bonus;

		if (dtype & D_MAGICAL)
			bonus = BLESS_MAGIC_BONUS;
		else 
			bonus = BLESS_BONUS;

		Resistance = min(Resistance, 1/bonus);
	}

	//Check for salvation
	if ((aura = que_findtype(targ->auras, aura, AURA_SALVATION)) != NULL)
	{
		// salvation gives heavy resistance against all magics and summonables
		// 60% magic resistance, 40% against other attacks
		if ((dtype & D_MAGICAL) || attacker->activator || attacker->creator)
			temp = 1 + 0.15*aura->ent->owner->myskills.abilities[SALVATION].current_level;
		else // it's a player doing the damage
			temp = 1 + 0.066*aura->ent->owner->myskills.abilities[SALVATION].current_level;

		Resistance = min(Resistance, 1/temp);
	}

	if (targ->client)
	{
		if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(targ))
			return damage; // special rules, flag carrier can't use abilities

		if (dtype & D_WORLD)
		{ 
			if(SPREE_WAR && SPREE_DUDE && (targ != SPREE_DUDE))
				return 0;	// no world damage if someone is warring
			else if ((targ->myskills.abilities[WORLD_RESIST].current_level > 0) && (!targ->myskills.abilities[WORLD_RESIST].disable))
				return 0;	// no world damage if you have world resist
			else if ((isMorphingPolt(targ)) && (targ->mtype == 0))
				return 0;	//Poltergeists can not take world damage in human form
		}
		if ((targ == attacker) && (mod == MOD_BOMBS))
			return 0; // cannot bomb yourself

		if (targ->mtype)
			return damage; // morphed players can't use abilities










		//Talent: Blood of Ares
		if (getTalentSlot(targ, TALENT_BLOOD_OF_ARES) != -1)
		{
			int level = getTalentLevel(targ, TALENT_BLOOD_OF_ARES);
			float temp;
			
			// BoA is more effective in PvM
			if (pvm->value || invasion->value)
				temp = level * 0.02 *attacker->myskills.streak;  //  from 0.01 to 0.02
			else
				temp = level * 0.01 *attacker->myskills.streak;

			//Limit bonus to +150%
			if(temp > 1.5)	temp = 1.5;
			
			damage *= 1.0 + temp;
		}

		// resistance effect
		if (!targ->myskills.abilities[RESISTANCE].disable)
		{
			if (!V_IsPVP() || !ffa->value)
				temp = 1 + 0.1 * targ->myskills.abilities[RESISTANCE].current_level;
			// PvP modes are getting frustrating with players that are too resisting   0.1 in pvp should be fine.
			else if ( (!pvm->value && !invasion->value) )
				temp = 1 + 0.1 * targ->myskills.abilities[RESISTANCE].current_level;

			//Talent: Improved Resist in 
			//talentLevel  = getTalentLevel(targ, TALENT_IMP_RESIST);
			//if(talentLevel > 0)		
				//temp += talentLevel * 0.1;

			//Talent: Improved Strength
			talentLevel = getTalentLevel(targ, TALENT_IMP_STRENGTH);
			if(talentLevel > 0)		
				temp += talentLevel * 0.02;
			
			// don't allow more than 100% damage
			//if (temp < 1.0)
				//temp = 1.0;

			Resistance = min(Resistance, 1/temp);
		}

		//Talent: Combat Experience
		talentLevel = getTalentLevel(targ, TALENT_COMBAT_EXP);
		if(talentLevel > 0)
			damage *= 1.0 + 0.05 * talentLevel;	//-5% per upgrade

		// ghost effect
		if ((!targ->myskills.abilities[GHOST].disable) && !(dflags & DAMAGE_NO_PROTECTION)) 
		{
			int talentSlot = getTalentSlot(targ, TALENT_SECOND_CHANCE);
			temp = 1 + 0.035*targ->myskills.abilities[GHOST].current_level;
			temp = 1.0/temp;

			// cap ghost resistance to 75%
			if (isMorphingPolt(targ))
				temp = 0.25;
			
			if (temp < 0.4 && !isMorphingPolt(targ))
			{
				if (!pvm->value && !invasion->value) // PVP mode? cap it to 40%.
					temp = 0.4;
			}

			if (!hw->value)
			{
				if (random() >= temp)
					return 0;
			}else
			{
				// Doesn't have the halo? Have ghost. (az remainder: this is if you do have ghost)
				if (dclient && !dclient->client->pers.inventory[halo_index])
					if (random() >= temp)
						return 0;
			}

			//Talent: Second Chance
			if(talentSlot != -1)
			{
				talent_t *talent = &targ->myskills.talents.talent[talentSlot];
				
				//Make sure the talent is not on cooldown
				if(talent->upgradeLevel > 0 && talent->delay < level.time)
				{
					//Cooldown should be 3 minutes - 0.5min per upgrade level
					float cooldown = 180 - 30 * getTalentLevel(targ, TALENT_SECOND_CHANCE);
					talent->delay = level.time + cooldown;
					return 0;
				}
			}

		}

		// grouped weapon resists
		if ((dtype & D_EXPLOSIVE) && (targ->myskills.abilities[SPLASH_RESIST].current_level > 0))
		{
			if(!targ->myskills.abilities[SPLASH_RESIST].disable)
			{
				Resistance = min(Resistance, 0.4);
			}
		}
		else if ((dtype & D_PIERCING) && (targ->myskills.abilities[PIERCING_RESIST].current_level > 0))
		{
			if(!targ->myskills.abilities[PIERCING_RESIST].disable)
				Resistance = min(Resistance, 0.4);
		}
		else if ((dtype & D_ENERGY) && (targ->myskills.abilities[ENERGY_RESIST].current_level > 0))
		{
			if(!targ->myskills.abilities[ENERGY_RESIST].disable)
				Resistance = min(Resistance, 0.4);
		}
		else if ((dtype & D_SHELL) && (targ->myskills.abilities[SHELL_RESIST].current_level > 0))
		{
			if(!targ->myskills.abilities[SHELL_RESIST].disable)
				Resistance = min(Resistance, 0.4);
		}
		else if ((dtype & D_BULLET) && (targ->myskills.abilities[BULLET_RESIST].current_level > 0))
		{
			if(!targ->myskills.abilities[BULLET_RESIST].disable)
				Resistance = min(Resistance, 0.4);
		}

		//Talent: Manashield   
		if(getTalentLevel(targ, TALENT_MANASHIELD) > 0 && targ->manashield)   
		{    
			int damage_absorbed = 0.8 * damage;    
			float absorb_mult = 3.5 - 0.5668 * getTalentLevel(targ, TALENT_MANASHIELD);    
			int pc_cost = damage_absorbed * absorb_mult;    
			int *cubes = &targ->client->pers.inventory[power_cube_index];   

			if (pc_cost > *cubes)    
			{     
				//too few cubes, so use them all up     
				damage_absorbed = *cubes / absorb_mult;     
				*cubes = 0;                 
				damage -= damage_absorbed;     
				targ->manashield = false;    
			}    
			else    
			{    
				//we have enough cubes to absorb all of the damage     
				damage -= damage_absorbed;     *cubes -= pc_cost;    
			}   
		}

		//4.1 Fury ability reduces damage.
		if(targ->fury_time > level.time)
		{
			// (apple)
			// Changed to attacker instead of target's fury level!
			temp = FURY_INITIAL_FACTOR + (FURY_ADDON_FACTOR * attacker->myskills.abilities[FURY].current_level);
			if (temp > FURY_FACTOR_MAX)
				temp = FURY_FACTOR_MAX;
			Resistance = min(Resistance, 1/temp);
		}
	}

	talentLevel = getTalentLevel(targ, TALENT_BLAST_RESIST);

	if (talentLevel && ((dflags & DAMAGE_RADIUS) || mod == MOD_SELFDESTRUCT))
	{
		temp = 1 - 0.15 * talentLevel;
		Resistance = min(Resistance, temp);
	}

	damage *= Resistance;

	// gi.dprintf("%f damage after G_SubDamage() modification (%f resistance mult)\n", damage, Resistance);

	return damage;
}

float G_ModifyDamage (edict_t *targ, edict_t *inflictor, edict_t *attacker, 
				   vec3_t point, float damage, int dflags, int mod)
{
	int		pierceLevel=0, pierceFactor;
	float	temp;

	if (damage > 0)
	{
		if (attacker->client)
		{
			// these weapons have armor-piercing capabilities
			if (mod == MOD_MACHINEGUN)
			{
				// 25% chance at level 10 for AP round
				pierceLevel = attacker->myskills.weapons[WEAPON_MACHINEGUN].mods[1].current_level;
				pierceFactor = 0.0333;
			}
			else if (mod == MOD_RAILGUN)
			{
				// 10% chance at level 10 for AP round
				pierceLevel = attacker->myskills.weapons[WEAPON_RAILGUN].mods[1].current_level;
				pierceFactor = 0.0111;
			}

			if (pierceLevel > 0)
			{
				temp = 1 + pierceFactor * pierceLevel;
				temp = 1.0 / temp;

				if (random() >= temp)
					dflags |= DAMAGE_NO_ARMOR;
			}
		}

		// if we hit someone that is cloaked, uncloak them!
		if (targ->client && targ->client->cloaking)
			targ->client->idle_frames = 0;
	}

	return damage;
}
