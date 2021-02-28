#include "g_local.h"

//************************************************************************************************
//			Bless think
//************************************************************************************************

void Bless_think(edict_t *self)
{
	//Find my slot
	que_t *slot = NULL;
	slot = que_findtype(self->enemy->curses, NULL, BLESS);

	// Blessing self-terminates if the enemy dies or the duration expires
	if (!slot || !que_valident(slot))
	{
		self->enemy->superspeed = false;
		que_removeent(self->enemy->curses, self, true);
		return;
	}

	//Stick with the target
	VectorCopy(self->enemy->s.origin, self->s.origin);
	gi.linkentity(self);

	//give them super speed
	self->enemy->superspeed = true;

	//Next think
	self->nextthink = level.time + FRAMETIME;

}

//************************************************************************************************
//			Healing (Blessing) think
//************************************************************************************************

void Healing_think(edict_t *self)
{
	//Find my slot
	que_t *slot = NULL;	
	int heal_amount = HEALING_HEAL_BASE + HEALING_HEAL_BONUS * self->owner->myskills.abilities[HEALING].current_level;
	float cooldown = 1.0;

	slot = que_findtype(self->enemy->curses, NULL, HEALING);

	// Blessing self-terminates if the enemy dies or the duration expires
	if (!slot || !que_valident(slot))
	{
		que_removeent(self->enemy->curses, self, true);
		return;
	}

	//Stick with the target
	VectorCopy(self->enemy->s.origin, self->s.origin);
	gi.linkentity(self);

	//Next think time
	self->nextthink = level.time + cooldown;

	//Heal the target's armor
	if (!self->enemy->client)
	{
		//Check to make sure it's a monster
		if (!self->enemy->mtype)
			return;
		
		heal_amount = self->enemy->max_health * (0.01 * self->owner->myskills.abilities[HEALING].current_level); // 1% healed per level
		
		if (heal_amount > 100)
			heal_amount = 100;

		//Heal the momster's health
		self->enemy->health += heal_amount;
		if (self->enemy->health > self->enemy->max_health)
			self->enemy->health = self->enemy->max_health;

		if (self->enemy->monsterinfo.power_armor_type)
		{
			heal_amount = self->enemy->monsterinfo.power_armor_power * (0.01 * self->owner->myskills.abilities[HEALING].current_level); // 1% healed per level
			
			if (heal_amount > 100)
				heal_amount = 100;

			//Heal the monster's armor
			self->enemy->monsterinfo.power_armor_power += heal_amount;
			if (self->enemy->monsterinfo.power_armor_power > self->enemy->monsterinfo.max_armor)
				self->enemy->monsterinfo.power_armor_power = self->enemy->monsterinfo.max_armor;
		}
	}
	else
	{
		if (self->enemy->health < MAX_HEALTH(self->enemy))
		{
			//Heal health
			self->enemy->health += heal_amount;
			if (self->enemy->health > MAX_HEALTH(self->enemy))
				self->enemy->health = MAX_HEALTH(self->enemy);
		}

		if (self->enemy->client->pers.inventory[body_armor_index] < MAX_ARMOR(self->enemy))
		{
			//Heal armor
			heal_amount *= 0.5; // don't heal as much armor
			if (heal_amount < 1)
				heal_amount = 1;
			self->enemy->client->pers.inventory[body_armor_index] += heal_amount;
			if (self->enemy->client->pers.inventory[body_armor_index] > MAX_ARMOR(self->enemy))
				self->enemy->client->pers.inventory[body_armor_index] = MAX_ARMOR(self->enemy);
		}
	}
}

//************************************************************************************************
//			Generic Curse functions
//************************************************************************************************

qboolean G_CurseValidTarget (edict_t *self, edict_t *target, qboolean vis, qboolean isCurse)
{
	if (!G_EntIsAlive(target))
		return false;
	// don't target players with invulnerability
	if (target->client && (target->client->invincible_framenum > level.framenum))
		return false;
	// don't target spawning players
	if (target->client && (target->client->respawn_time > level.time))
		return false;
	// don't target players in chat-protect
	if (!ptr->value && target->client && (target->flags & FL_CHATPROTECT))
		return false;
	// don't target spawning world monsters
	if (target->activator && !target->activator->client && (target->svflags & SVF_MONSTER) 
		&& (target->deadflag != DEAD_DEAD) && (target->nextthink-level.time > 2*FRAMETIME))
		return false;
	// don't target cloaked players
	if (target->client && target->svflags & SVF_NOCLIENT)
		return false;
	if (vis && !visible(self, target))
		return false;
	if(que_typeexists(target->curses, CURSE_FROZEN))
		return false;
	if (isCurse && (target->flags & FL_GODMODE || OnSameTeam(self, target)))
		return false;
	if (target == self)
		return false;
	
	return true;
}

void curse_think(edict_t *self)
{
	//Find my curse slot
	que_t *slot = NULL;
	slot = que_findtype(self->enemy->curses, NULL, self->atype);

	// curse self-terminates if the enemy dies or the duration expires
	if (!slot || !que_valident(slot))
	{
		que_removeent(self->enemy->curses, self, true);
		return;
	}

	CurseEffects(self->enemy, 10, 242);

	//Stick with the target
	VectorCopy(self->enemy->s.origin, self->s.origin);
	gi.linkentity(self);

	//Next think time
	self->nextthink = level.time + FRAMETIME;

	LifeDrain(self);// 3.5 this must be called last, because it may free the curse ent
	Bleed(self);//4.2
}

//************************************************************************************************

qboolean curse_add(edict_t *target, edict_t *caster, int type, int curse_level, float duration)
{
	edict_t *curse;
	que_t	*slot = NULL;
	
	if (type != BLESS && type != HEALING)
		if (target == caster)
			return false;

	//Find out if this curse already exists
	slot = que_findtype(target->curses, NULL, type);
	if(slot != NULL)
	{
		//If the current curse in effect has a level greater than the caster's curse level
		//if (slot->ent->owner->myskills.abilities[type].current_level > caster->myskills.abilities[type].current_level)
		if (slot->ent->monsterinfo.level > curse_level)//4.4
			//Can't re-curse this player
			return false;
		else
		{
            //Refresh the curse with the new level/ent/duration
			return que_addent(target->curses, slot->ent, duration);
		}
	}

	/*
	//Talent: Evil curse (improves curse duration)
	talentLevel = getTalentLevel(caster, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
	{
		//Curses only
		if(type != BLESS && type != HEALING)
			duration *= 1.0 + 0.25 * talentLevel;
	}
	*/

	//Create the curse entity
	curse=G_Spawn();
	curse->classname = "curse";
	curse->solid = SOLID_NOT;
	curse->svflags |= SVF_NOCLIENT;
	curse->monsterinfo.level = curse_level;//4.2
	curse->monsterinfo.selected_time = duration;//4.2
	VectorClear(curse->velocity);
	VectorClear(curse->mins);
	VectorClear(curse->maxs);

	//Set curse type, target, and caster
	curse->owner = caster;
	curse->enemy = target;
	curse->atype = type;

	//First think in 1/2 a second
	curse->nextthink = level.time + FRAMETIME;
	curse->think = curse_think;

	//Set origin to target's origin
	VectorCopy(target->s.origin, curse->s.origin);
	gi.linkentity(curse);

	//Try to add the curse to the que
	if (!que_addent(target->curses, curse, duration))
	{
		G_FreeEdict(curse);
		return false;
	}
	return true;
}

//************************************************************************************************

// find valid targets that are within range and field of vision
edict_t *curse_MultiAttack (edict_t *e, edict_t *caster, int type, int range, float duration, qboolean isCurse)
{
	//edict_t *e=NULL;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (caster->s.origin);
	gi.multicast (caster->s.origin, MULTICAST_PVS);

	caster->client->idle_frames = 0; // disable cp/cloak on caster
	caster->client->ability_delay = level.time + 0.2; // avoid spell spam

	while ((e = findreticle(e, caster, range, 20, true)) != NULL)
	{
		if (!G_CurseValidTarget(caster, e, false, isCurse))
			continue;
		// holywater gives immunity to curses for a short time.
		if (e->holywaterProtection > level.time && type != BLESS && type != HEALING)
			continue;
		// don't allow bless on flag carrier
		if ((type == BLESS) && e->client && HasFlag(e))
			continue;
		if (!curse_add(e, caster, type, 0, duration))
			continue;
		return e;
	}
	return NULL;
}

// find a single valid target that is in-range and nearest to the aiming reticle
edict_t *curse_Attack(edict_t *caster, int type, int radius, float duration, qboolean isCurse)
{
	edict_t *e=NULL;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (caster->s.origin);
	gi.multicast (caster->s.origin, MULTICAST_PVS);

	caster->client->idle_frames = 0; // disable cp/cloak on caster
	caster->client->ability_delay = level.time + 0.2; // avoid spell spam

	while ((e = findclosestreticle(e, caster, radius)) != NULL)
	{
		if (!G_CurseValidTarget(caster, e, true, isCurse))
			continue;
	//	if (entdist(caster, e) > radius)
	//		continue;
		//4.0 holywater gives immunity to curses for a short time.
		if (e->holywaterProtection > level.time && type != BLESS && type != HEALING)
			continue;
		// 3.7 don't allow bless on flag carrier
		if ((type == BLESS) && e->client && HasFlag(e))
			continue;
		if (!curse_add(e, caster, type, 0, duration))
			continue;
		return e;
	}
	return NULL;
}

qboolean CanCurseTarget (edict_t *caster, edict_t *target, int type, qboolean isCurse, qboolean vis)
{
	if (!G_EntIsAlive(target))
		return false;
	// don't target players with invulnerability
	if (target->client && (target->client->invincible_framenum > level.framenum))
		return false;
	// don't target spawning players
	if (target->client && (target->client->respawn_time > level.time))
		return false;
	// don't target players in chat-protect
	if (!ptr->value && target->client && (target->flags & FL_CHATPROTECT))
		return false;
	// don't target spawning world monsters
	if (target->activator && !target->activator->client && (target->svflags & SVF_MONSTER) 
		&& (target->deadflag != DEAD_DEAD) && (target->nextthink-level.time > 2*FRAMETIME))
		return false;
	// don't target cloaked players
	if (target->client && target->svflags & SVF_NOCLIENT)
		return false;
	if (vis && !visible(caster, target))
		return false;
	if(que_typeexists(target->curses, CURSE_FROZEN))
		return false;
	if (isCurse && (target->flags & FL_GODMODE || OnSameTeam(caster, target)))
		return false;
	if (target == caster)
		return false;
	// holywater gives immunity to curses for a short time.
	if (target->holywaterProtection > level.time && type != BLESS && type != HEALING)
		return false;
	// don't allow bless on flag carrier
	if ((type == BLESS) && target->client && HasFlag(target))
		return false;
	return true;
}

char *GetCurseName (int type)
{
	switch (type)
	{
	case HEALING: return "healing";
	case BLESS: return "bless";
	case DEFLECT: return "deflect";
	case CURSE: return "confuse";
	case LOWER_RESIST: return "lower resist";
	case AMP_DAMAGE: return "amp damage";
	case WEAKEN: return "weaken";
	case LIFE_DRAIN: return "life drain";
	default: return "";
	}
}

void CurseMessage (edict_t *caster, edict_t *target, int type, float duration, qboolean isCurse)
{
	char *curseName = GetCurseName(type);
	char *typeName;

	if (isCurse)
		typeName = "cursed";
	else
		typeName = "blessed";

	//Notify the target
	if ((target->client) && !(target->svflags & SVF_MONSTER))
	{
		safe_cprintf(target, PRINT_HIGH, "**You have been %s with %s for %0.1f second(s)**\n", typeName, curseName, duration);
		if (caster && caster->client)
			safe_cprintf(caster, PRINT_HIGH, "%s %s with %s for %0.1f second(s)\n", typeName, target->myskills.player_name, curseName, duration);
	}
	else if (target->mtype)
	{
		if (PM_MonsterHasPilot(target))
		{
			safe_cprintf(target->activator, PRINT_HIGH, "**You have been %s with %s for %0.1f second(s)**\n", typeName, curseName, duration);
			if (caster && caster->client)
				safe_cprintf(caster, PRINT_HIGH, "%s %s with %s for %0.1f second(s)\n", typeName, target->activator->client->pers.netname, curseName, duration);
			return;
		}

		if (caster && caster->client)
			safe_cprintf(caster, PRINT_HIGH, "%s %s with %s for %0.1f second(s)\n", typeName, V_GetMonsterName(target), curseName, duration);
	}
	else if (caster && caster->client)
		safe_cprintf(caster, PRINT_HIGH, "%s %s with %s for %0.1f second(s)\n", typeName, target->classname, curseName, duration);
}

void CurseRadiusAttack (edict_t *caster, int type, int range, int radius, float duration, qboolean isCurse)
{
	edict_t *e=NULL, *f=NULL;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (caster->s.origin);
	gi.multicast (caster->s.origin, MULTICAST_PVS);

	caster->client->idle_frames = 0;
	caster->client->ability_delay = level.time;// for monster hearing (check if ability was recently used/cast)

	// find a target closest to the caster's reticle
	while ((e = findclosestreticle(e, caster, range)) != NULL)
	{
		if (!CanCurseTarget(caster, e, type, isCurse, true))
			continue;
		if (entdist(caster, e) > range)
			continue;
		if (!infront(caster, e))
			continue;
		if (!curse_add(e, caster, type, 0, duration))
			continue;
		CurseMessage(caster, e, type, duration, isCurse);

		// target anything in-range of this entity
		while ((f = findradius(f, e->s.origin, radius)) != NULL)
		{
			if (!CanCurseTarget(caster, f, type, isCurse, false))
				continue;
			if (f == e)
				continue;
			if (!visible(e, f))
				continue;
			if (!curse_add(f, caster, type, 0, duration))
				continue;
			CurseMessage(caster, f, type, duration, isCurse);
		}

		break;
	}
}

//************************************************************************************************
//			Lower Resist Curse
//************************************************************************************************

void Cmd_LowerResist (edict_t *ent)
{
	int range, radius, talentLevel, cost=LOWER_RESIST_COST;
	float duration;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_LowerResist()\n", ent->client->pers.netname);

	//Talent: Cheaper Curses
	if ((talentLevel = getTalentLevel(ent, TALENT_CHEAPER_CURSES)) > 0)
		cost *= 1.0 - 0.1 * talentLevel;

	if (!V_CanUseAbilities(ent, LOWER_RESIST, cost, true))
		return;

	range = LOWER_RESIST_INITIAL_RANGE + LOWER_RESIST_ADDON_RANGE * ent->myskills.abilities[LOWER_RESIST].current_level;
	radius = LOWER_RESIST_INITIAL_RADIUS + LOWER_RESIST_ADDON_RADIUS * ent->myskills.abilities[LOWER_RESIST].current_level;
	duration = LOWER_RESIST_INITIAL_DURATION + LOWER_RESIST_ADDON_DURATION * ent->myskills.abilities[LOWER_RESIST].current_level;

	// evil curse talent
	talentLevel = getTalentLevel(ent, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
		duration *= 1.0 + 0.25 * talentLevel;

	if (duration < 1)
		duration = 1;

	CurseRadiusAttack(ent, LOWER_RESIST, range, radius, duration, true);

	//Finish casting the spell
	//ent->client->ability_delay = level.time + LOWER_RESIST_DELAY;
	ent->myskills.abilities[LOWER_RESIST].delay = level.time + LOWER_RESIST_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;

	//Play the spell sound!
	gi.sound(ent, CHAN_ITEM, gi.soundindex("curses/lowerresist.wav"), 1, ATTN_NORM, 0);

}

//************************************************************************************************
//			Amp Damage Curse
//************************************************************************************************

void Cmd_AmpDamage(edict_t *ent)
{
	int range, radius, talentLevel, cost=AMP_DAMAGE_COST;
	float duration;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_AmpDamage()\n", ent->client->pers.netname);

	//Talent: Cheaper Curses
	if ((talentLevel = getTalentLevel(ent, TALENT_CHEAPER_CURSES)) > 0)
		cost *= 1.0 - 0.1 * talentLevel;

	if (!V_CanUseAbilities(ent, AMP_DAMAGE, cost, true))
		return;

	range = CURSE_DEFAULT_INITIAL_RANGE + CURSE_DEFAULT_ADDON_RANGE * ent->myskills.abilities[AMP_DAMAGE].current_level;
	radius = CURSE_DEFAULT_INITIAL_RADIUS + CURSE_DEFAULT_ADDON_RADIUS * ent->myskills.abilities[AMP_DAMAGE].current_level;
	duration = AMP_DAMAGE_DURATION_BASE + AMP_DAMAGE_DURATION_BONUS * ent->myskills.abilities[AMP_DAMAGE].current_level;

	//Talent: Evil curse
	talentLevel = getTalentLevel(ent, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
		duration *= 1.0 + 0.25 * talentLevel;

	if (duration < 1)
		duration = 1;

	CurseRadiusAttack(ent, AMP_DAMAGE, range, radius, duration, true);
	
	//Finish casting the spell
	//ent->client->ability_delay = level.time + AMP_DAMAGE_DELAY;
	ent->myskills.abilities[AMP_DAMAGE].delay = level.time + AMP_DAMAGE_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;

	//Play the spell sound!
	gi.sound(ent, CHAN_ITEM, gi.soundindex("curses/amplifydamage.wav"), 1, ATTN_NORM, 0);
}

//************************************************************************************************
//			Curse
//************************************************************************************************

void Cmd_Curse(edict_t *ent)
{
	int range, radius, talentLevel, cost=CURSE_COST;
	float duration;
	edict_t *target = NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Curse()\n", ent->client->pers.netname);

	//Talent: Cheaper Curses
	if ((talentLevel = getTalentLevel(ent, TALENT_CHEAPER_CURSES)) > 0)
		cost *= 1.0 - 0.1 * talentLevel;

	if (!V_CanUseAbilities(ent, CURSE, cost, true))
		return;

	range = CURSE_DEFAULT_INITIAL_RANGE + CURSE_DEFAULT_ADDON_RANGE * ent->myskills.abilities[CURSE].current_level;
	radius = CURSE_DEFAULT_INITIAL_RADIUS + CURSE_DEFAULT_ADDON_RADIUS * ent->myskills.abilities[CURSE].current_level;
	duration = CURSE_DURATION_BASE + (CURSE_DURATION_BONUS * ent->myskills.abilities[CURSE].current_level);

	//Talent: Evil curse
	talentLevel = getTalentLevel(ent, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
		duration *= 1.0 + 0.25 * talentLevel;

	if (duration < 1)
		duration = 1;

	CurseRadiusAttack(ent, CURSE, range, radius, duration, true);

	//Finish casting the spell
	//ent->client->ability_delay = level.time + CURSE_DELAY;
	ent->myskills.abilities[CURSE].delay = level.time + CURSE_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;

	target = curse_Attack(ent, CURSE, radius, duration, true);

	//Play the spell sound!
	gi.sound(ent, CHAN_ITEM, gi.soundindex("curses/curse.wav"), 1, ATTN_NORM, 0);
	
}

//************************************************************************************************
//			Weaken Curse
//************************************************************************************************

void Cmd_Weaken(edict_t *ent)
{
	int range, radius, talentLevel, cost=WEAKEN_COST;
	float duration;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Weaken()\n", ent->client->pers.netname);

	//Talent: Cheaper Curses
	if ((talentLevel = getTalentLevel(ent, TALENT_CHEAPER_CURSES)) > 0)
		cost *= 1.0 - 0.1 * talentLevel;

	if (!V_CanUseAbilities(ent, WEAKEN, cost, true))
		return;

	range = CURSE_DEFAULT_INITIAL_RANGE + CURSE_DEFAULT_ADDON_RANGE * ent->myskills.abilities[WEAKEN].current_level;
	radius = CURSE_DEFAULT_INITIAL_RADIUS + CURSE_DEFAULT_ADDON_RADIUS * ent->myskills.abilities[WEAKEN].current_level;
	duration = WEAKEN_DURATION_BASE + (WEAKEN_DURATION_BONUS * ent->myskills.abilities[WEAKEN].current_level);

	//Talent: Evil curse
	talentLevel = getTalentLevel(ent, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
		duration *= 1.0 + 0.25 * talentLevel;

	if (duration < 1)
		duration = 1;

	CurseRadiusAttack(ent, WEAKEN, range, radius, duration, true);
	
	//Finish casting the spell
	//ent->client->ability_delay = level.time + WEAKEN_DELAY;
	ent->myskills.abilities[WEAKEN].delay = level.time + WEAKEN_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;

	//Play the spell sound!
	gi.sound(ent, CHAN_ITEM, gi.soundindex("curses/weaken.wav"), 1, ATTN_NORM, 0);
}

//************************************************************************************************
//			Life Drain Curse
//************************************************************************************************

void Cmd_LifeDrain(edict_t *ent)
{
	int		range, radius, talentLevel, cost=LIFE_DRAIN_COST;
	float	duration;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_LifeDrain()\n", ent->client->pers.netname);

	//Talent: Cheaper Curses
	if ((talentLevel = getTalentLevel(ent, TALENT_CHEAPER_CURSES)) > 0)
		cost *= 1.0 - 0.1 * talentLevel;

	if (!V_CanUseAbilities(ent, LIFE_DRAIN, cost, true))
		return;

	range = CURSE_DEFAULT_INITIAL_RANGE + CURSE_DEFAULT_ADDON_RANGE * ent->myskills.abilities[LIFE_DRAIN].current_level;
	radius = CURSE_DEFAULT_INITIAL_RADIUS + CURSE_DEFAULT_ADDON_RADIUS * ent->myskills.abilities[LIFE_DRAIN].current_level;
	duration = LIFE_DRAIN_DURATION_BASE + LIFE_DRAIN_DURATION_BONUS * ent->myskills.abilities[LIFE_DRAIN].current_level;

	//Talent: Evil curse
	talentLevel = getTalentLevel(ent, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
		duration *= 1.0 + 0.25 * talentLevel;

	if (duration < 1)
		duration = 1;

	CurseRadiusAttack(ent, LIFE_DRAIN, range, radius, duration, true);
	
	//Finish casting the spell
	//ent->client->ability_delay = level.time + LIFE_DRAIN_DELAY;
	ent->myskills.abilities[LIFE_DRAIN].delay = level.time + LIFE_DRAIN_DELAY;
	ent->client->pers.inventory[power_cube_index] -= cost;

	//Play the spell sound!
	gi.sound(ent, CHAN_ITEM, gi.soundindex("curses/ironmaiden.wav"), 1, ATTN_NORM, 0);// FIXME: change this!
}

void Bleed (edict_t *curse)
{
	int		take;
	edict_t *caster=curse->owner;

	if (curse->atype != BLEEDING)
		return;

	if (level.time < curse->wait)
		return;

	if (!G_ValidTarget(caster, curse->enemy, false))
	{
		// remove the curse if the target dies
		que_removeent(curse->enemy->curses, curse, true);
		return;
	}

	// 33-99% health taken over duration of curse
	take = (curse->enemy->max_health * (0.033 * curse->monsterinfo.level)) / curse->monsterinfo.selected_time;

	//gi.dprintf("target %s take %d health %d/%d level %d time %.1f\n", 
	//	curse->enemy->classname, take, curse->enemy->health, curse->enemy->max_health,
	//	curse->monsterinfo.level, curse->monsterinfo.selected_time);

	// damage limits
	if (take < 1)
		take = 1;
	if (take > 100)
		take = 100;

	T_Damage(curse->enemy, caster, caster, vec3_origin, vec3_origin, 
		vec3_origin, take, 0, DAMAGE_NO_ABILITIES, MOD_LIFE_DRAIN);

	curse->wait = level.time + (GetRandom(3, 10) * FRAMETIME);
}

void LifeDrain (edict_t *curse)
{
	int		take;
	edict_t *caster=curse->owner;

	if (curse->atype != LIFE_DRAIN)
		return;

	if (level.time < curse->wait)
		return;

	if (!G_ValidTarget(caster, curse->enemy, false))
	{
		// remove the curse if the target dies
		que_removeent(curse->enemy->curses, curse, true);
		return;
	}

	take = LIFE_DRAIN_HEALTH;
	// more effective on non-clients (because they have more health)
	if (!curse->enemy->client)
		take *= 2;

	// give caster health
	if (caster->health < caster->max_health)
	{
		caster->health += take;
		if (caster->health > caster->max_health)
			caster->health = caster->max_health;
	}

	// take it away from curse's target
	if (!curse->enemy->client) //deal even MORE damage to non-players.
		T_Damage(curse->enemy, caster, caster, vec3_origin, vec3_origin, 
			vec3_origin, take * 1.5, 0, DAMAGE_NO_ABILITIES, MOD_LIFE_DRAIN);
	else
		T_Damage(curse->enemy, caster, caster, vec3_origin, vec3_origin, 
			vec3_origin, take, 0, DAMAGE_NO_ABILITIES, MOD_LIFE_DRAIN);

	curse->wait = level.time + LIFE_DRAIN_UPDATETIME;
}

//************************************************************************************************
//			Disables all skills, called when target is cursed with amnesia
//************************************************************************************************

void V_DisableAllSkills(edict_t *ent)
{
	//jetpack
	ent->client->thrusting = 0;
	//grapple hook
	ent->client->hook_state = HOOK_READY;
	//power screen
	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	//superspeed
	ent->superspeed = false;
	ent->antigrav = false;
	//Disable all auras
	AuraRemove(ent, 0);
}

//************************************************************************************************
//			Amnesia Curse
//************************************************************************************************

void Cmd_Amnesia(edict_t *ent)
{
	int radius, talentLevel, cost=AMNESIA_COST;
	float duration;
	edict_t *target = NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Amnesia()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[AMNESIA].disable)
		return;

	//Talent: Cheaper Curses
	if ((talentLevel = getTalentLevel(ent, TALENT_CHEAPER_CURSES)) > 0)
		cost *= 1.0 - 0.1 * talentLevel;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[AMNESIA].current_level, cost))
		return;

	radius = SHAMAN_CURSE_RADIUS_BASE + (SHAMAN_CURSE_RADIUS_BONUS * ent->myskills.abilities[AMNESIA].current_level);
	duration = AMNESIA_DURATION_BASE + (AMNESIA_DURATION_BONUS * ent->myskills.abilities[AMNESIA].current_level);

	//Talent: Evil curse
	talentLevel = getTalentLevel(ent, TALENT_EVIL_CURSE);
	if(talentLevel > 0)
		duration *= 1.0 + 0.25 * talentLevel;

	target = curse_Attack(ent, AMNESIA, radius, duration, true);
	if (target != NULL)
	{
		//Finish casting the spell
		ent->client->ability_delay = level.time + AMNESIA_DELAY;
		ent->client->pers.inventory[power_cube_index] -= cost;

		//disable some abilities if the player was using them
		V_DisableAllSkills(ent);

		//Notify the target
		if ((target->client) && !(target->svflags & SVF_MONSTER))
		{
			safe_cprintf(target, PRINT_HIGH, "YOU HAVE BEEN CURSED WITH AMNESIA!! (%0.1f seconds)\n", duration);
			safe_cprintf(ent, PRINT_HIGH, "Cursed %s with amnesia for %0.1f seconds.\n", target->myskills.player_name, duration);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Cursed %s with amnesia for %0.1f seconds.\n", target->classname, duration);
		}
		//Play the spell sound!
		gi.sound(target, CHAN_ITEM, gi.soundindex("curses/amnesia.wav"), 1, ATTN_NORM, 0);
	}
}

//************************************************************************************************
//			Healing (Blessing)
//************************************************************************************************

void Cmd_Healing(edict_t *ent)
{
	int radius;
	float duration;
	edict_t *target = NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Healing()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[HEALING].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[HEALING].current_level, HEALING_COST))
		return;

	radius = SHAMAN_CURSE_RADIUS_BASE + (SHAMAN_CURSE_RADIUS_BONUS * ent->myskills.abilities[HEALING].current_level);
	duration = HEALING_DURATION_BASE + (HEALING_DURATION_BONUS * ent->myskills.abilities[HEALING].current_level);

	//Blessing self?
	if (Q_strcasecmp(gi.argv(1), "self") == 0)
	{
		if (!curse_add(ent, ent, HEALING, 0, duration))
		{
			safe_cprintf(ent, PRINT_HIGH, "Unable to bless self.\n");
			return;
		}
		target = ent;
	}
	else
	{
		target = curse_Attack(ent, HEALING, radius, duration, false);
	}
	if (target != NULL)
	{
		que_t *slot = NULL;
		//Finish casting the spell
		ent->client->ability_delay = level.time + HEALING_DELAY;
		ent->client->pers.inventory[power_cube_index] -= HEALING_COST;

		//Change the curse think to the healing think
		slot = que_findtype(target->curses, NULL, HEALING);
		if (slot)
		{
			slot->ent->think = Healing_think;
			slot->ent->nextthink = level.time + FRAMETIME;
		}

		//Notify the target
		if (target == ent)
		{
			safe_cprintf(target, PRINT_HIGH, "YOU HAVE BEEN BLESSED WITH %0.1f seconds OF HEALING!!\n", duration);
		}
		else if ((target->client) && !(target->svflags & SVF_MONSTER))
		{
			safe_cprintf(target, PRINT_HIGH, "YOU HAVE BEEN BLESSED WITH %0.1f seconds OF HEALING!!\n", duration);
			safe_cprintf(ent, PRINT_HIGH, "Blessed %s with healing for %0.1f seconds.\n", target->myskills.player_name, duration);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Blessed %s with healing for %0.1f seconds.\n", target->classname, duration);
		}
		//Play the spell sound!
		gi.sound(target, CHAN_ITEM, gi.soundindex("curses/prayer.wav"), 1, ATTN_NORM, 0);
	}
}

//************************************************************************************************
//			Bless (Blessing)
//************************************************************************************************

void Cmd_Bless(edict_t *ent)
{
	int radius;
	float duration, cooldown;
	edict_t *target = NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Bless()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[BLESS].disable)
		return;

	//if (!G_CanUseAbilities(ent, ent->myskills.abilities[BLESS].current_level, BLESS_COST))
	//	return;

	if (!V_CanUseAbilities(ent, BLESS, BLESS_COST, true))
		return;

	radius = SHAMAN_CURSE_RADIUS_BASE + (SHAMAN_CURSE_RADIUS_BONUS * ent->myskills.abilities[BLESS].current_level);
	duration = BLESS_DURATION_BASE + (BLESS_DURATION_BONUS * ent->myskills.abilities[BLESS].current_level);

	//Blessing self?
	if (Q_strcasecmp(gi.argv(1), "self") == 0)
	{
		if (HasFlag(ent))
		{
			safe_cprintf(ent, PRINT_HIGH, "Can't use this while carrying the flag!\n");
			return;
		}

		if (!curse_add(ent, ent, BLESS, 0, duration))
		{
			safe_cprintf(ent, PRINT_HIGH, "Unable to bless self.\n");
			return;
		}
		target = ent;
	}
	else
	{
		target = curse_Attack(ent, BLESS, radius, duration, false);
	}

	if (target != NULL)
	{
		que_t *slot = NULL;

		//Finish casting the spell
		ent->client->ability_delay = level.time + BLESS_DELAY;
		ent->client->pers.inventory[power_cube_index] -= BLESS_COST;

		cooldown = 2.0 * duration;
		if (cooldown > 10.0)
			cooldown = 10.0;

		ent->myskills.abilities[BLESS].delay = level.time + cooldown;

		//Change the curse think to the bless think
		slot = que_findtype(target->curses, NULL, BLESS);
		if (slot)
		{
			slot->ent->think = Bless_think;
			slot->ent->nextthink = level.time + FRAMETIME;
		}

		//Notify the target
		if (target == ent)
		{
			safe_cprintf(target, PRINT_HIGH, "YOU HAVE BEEN BLESSED FOR %0.1f seconds!!\n", duration);
		}
		else if ((target->client) && !(target->svflags & SVF_MONSTER))
		{
			safe_cprintf(target, PRINT_HIGH, "YOU HAVE BEEN BLESSED FOR %0.1f seconds!!\n", duration);
			safe_cprintf(ent, PRINT_HIGH, "Blessed %s for %0.1f seconds.\n", target->myskills.player_name, duration);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Blessed %s for %0.1f seconds.\n", target->classname, duration);
		}
		//Play the spell sound!
		gi.sound(target, CHAN_ITEM, gi.soundindex("curses/bless.wav"), 1, ATTN_NORM, 0);
	}
}

//************************************************************************************************
//			Deflect (Blessing)
//************************************************************************************************

void DeflectProjectiles (edict_t *self, float chance, qboolean in_front);

void deflect_think (edict_t *self)
{
	edict_t *player = G_GetClient(self->enemy);
	//Find my slot
	que_t *slot = NULL;
	slot = que_findtype(self->enemy->curses, NULL, DEFLECT);

	// Blessing self-terminates if the enemy dies or the duration expires
	if (!slot || !que_valident(slot))
	{
		if (player && level.time >= self->monsterinfo.selected_time)
			safe_cprintf(player, PRINT_HIGH, "Deflect has expired\n");

		que_removeent(self->enemy->curses, self, true);
		return;
	}

	// warn player that deflect is about to expire
//	if (player && !(level.framenum % 10) && (level.time >= slot->time - 5))
//		safe_cprintf(player, PRINT_HIGH, "Deflect will expire in %.0f seconds\n", slot->time - level.time);

	//Stick with the target
	VectorCopy(self->enemy->s.origin, self->s.origin);
	gi.linkentity(self);

	DeflectProjectiles(self->enemy, self->random, false);

	//Next think
	self->nextthink = level.time + FRAMETIME;

}

void Cmd_Deflect_f(edict_t *ent)
{
	float duration;
	edict_t *target = ent; // default target is self

	if (!V_CanUseAbilities(ent, DEFLECT, DEFLECT_COST, true))
		return;

	duration = DEFLECT_INITIAL_DURATION + DEFLECT_ADDON_DURATION * ent->myskills.abilities[DEFLECT].current_level;

	// bless the tank instead of the noclipped player
	if (PM_PlayerHasMonster(ent))
		target = target->owner;

	//Blessing self?
	if (Q_strcasecmp(gi.argv(1), "self") == 0)
	{
		if (!curse_add(target, ent, DEFLECT, 0, duration))
		{
			safe_cprintf(ent, PRINT_HIGH, "Unable to bless self.\n");
			return;
		}
		//target = ent;
	}
	else
	{
		target = curse_Attack(ent, DEFLECT, 512.0, duration, false);
	}

	if (target != NULL)
	{
		que_t *slot = NULL;

		//Finish casting the spell
		ent->client->ability_delay = level.time + DEFLECT_DELAY;
		ent->client->pers.inventory[power_cube_index] -= DEFLECT_COST;
	//	ent->myskills.abilities[DEFLECT].delay = level.time + duration + DEFLECT_DELAY;

		//Change the curse think to the deflect think
		slot = que_findtype(target->curses, NULL, DEFLECT);
		if (slot)
		{
			slot->ent->think = deflect_think;
			slot->ent->nextthink = level.time + FRAMETIME;
			slot->ent->random = DEFLECT_INITIAL_PROJECTILE_CHANCE+DEFLECT_ADDON_HITSCAN_CHANCE*ent->myskills.abilities[DEFLECT].current_level;
			if (slot->ent->random > DEFLECT_MAX_PROJECTILE_CHANCE)
				slot->ent->random = DEFLECT_MAX_PROJECTILE_CHANCE;
		}

		//Notify the target
		if (target == ent)
		{
			safe_cprintf(target, PRINT_HIGH, "You have been blessed with deflect for %0.1f seconds!\n", duration);
		}
		else if ((target->client) && !(target->svflags & SVF_MONSTER))
		{
			safe_cprintf(target, PRINT_HIGH, "You have been blessed with deflect for %0.1f seconds!\n\n", duration);
			safe_cprintf(ent, PRINT_HIGH, "Blessed %s with deflect for %0.1f seconds.\n", target->myskills.player_name, duration);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Blessed %s with deflect for %0.1f seconds.\n", V_GetMonsterName(target), duration);
		}

		//Play the spell sound!
		gi.sound(target, CHAN_ITEM, gi.soundindex("curses/prayer.wav"), 1, ATTN_NORM, 0);
	}
}

//************************************************************************************************
//			Mind Absorb (passive skill)
//************************************************************************************************

void MindAbsorb(edict_t *ent) 
{  
	edict_t *target = NULL;  
	int radius;  
	int take;  
	int total;
	int abilityLevel = ent->myskills.abilities[MIND_ABSORB].current_level;   
	
	if(ent->myskills.abilities[MIND_ABSORB].disable)   
		return;   
	if (!V_CanUseAbilities(ent, MIND_ABSORB, 0, false))   
		return;   //Cloaking and chat protected players can't steal anything
	if ((ent->flags & FL_CHATPROTECT) || (ent->svflags & SVF_NOCLIENT))   
		return;   
	
	take = MIND_ABSORB_AMOUNT_BASE + (MIND_ABSORB_AMOUNT_BONUS * abilityLevel);  
	radius = MIND_ABSORB_RADIUS_BASE + (MIND_ABSORB_RADIUS_BONUS * abilityLevel);   
	
	// scan for targets  
	while ((target = findclosestradius(target, ent->s.origin, radius)) != NULL)  
	{   
		if (target == ent)    
			continue;   
		if (!G_ValidTarget(ent, target, true))    
			continue;   
		
		total = 0;
		
		if (target->client)
		{
			if (target->client->pers.inventory[power_cube_index] < take)
				total += target->client->pers.inventory[power_cube_index];
			else
				total += take;

			target->client->pers.inventory[power_cube_index] -= total;

			// a bit of amnesia too
			target->client->ability_delay = level.time + 0.1 * abilityLevel;  
		}
		else
		{
			if (target->health < take)
				total += target->health;
			else
				total += take;
		}

		//Cap cube count to max cubes 
		if (ent->client->pers.inventory[power_cube_index] + total < MAX_POWERCUBES(ent))
			ent->client->pers.inventory[power_cube_index] += total;
		else if (ent->client->pers.inventory[power_cube_index] < MAX_POWERCUBES(ent))
			ent->client->pers.inventory[power_cube_index] = MAX_POWERCUBES(ent); 

		// those powercubes hurt!  
		T_Damage(target, ent, ent, vec3_origin, vec3_origin, vec3_origin, total, 0, DAMAGE_NO_ARMOR, MOD_MINDABSORB);
	}  
}

#define CURSE_DIR_UP		1
#define CURSE_DIR_DOWN		2
#define CURSE_DIR_LEFT		3
#define CURSE_DIR_RIGHT		4
#define CURSE_DIR_LU		5
#define CURSE_DIR_LD		6
#define CURSE_DIR_RU		7
#define CURSE_DIR_RD		8
#define CURSE_MOVEMENT		GetRandom(1,4)
#define CURSE_MAX_ROLL		20

void CursedPlayer (edict_t *ent)
{
	int		i;
	vec3_t	forward;

	if (!G_EntIsAlive(ent))
		return;
	if (!ent->client)
		return;
	if (!que_typeexists(ent->curses, CURSE))
	{
		// reset roll angles
		ent->client->ps.pmove.delta_angles[ROLL] = 0;
		return;
	}

	if (level.time > ent->curse_delay)
	{
		ent->curse_dir = GetRandom(1, 8);
		ent->curse_delay = level.time + 2*random();
	}

	// copy current viewing angles
	VectorCopy(ent->client->v_angle, forward);

	// choose which direction to move angles
	switch (ent->curse_dir)
	{
	case 1: forward[PITCH]+=CURSE_MOVEMENT; break; // down
	case 2: forward[PITCH]-=CURSE_MOVEMENT; break; // up
	case 3: forward[YAW]+=CURSE_MOVEMENT; break; // left
	case 4: forward[YAW]-=CURSE_MOVEMENT; break; // right
	case 5: forward[YAW]+=CURSE_MOVEMENT; forward[PITCH]-=CURSE_MOVEMENT; break;
	case 6: forward[YAW]+=CURSE_MOVEMENT; forward[PITCH]+=CURSE_MOVEMENT; break;
	case 7: forward[YAW]-=CURSE_MOVEMENT; forward[PITCH]-=CURSE_MOVEMENT; break;
	case 8: forward[YAW]+=CURSE_MOVEMENT; forward[PITCH]+=CURSE_MOVEMENT; break;
	}

	// change roll angles
	if (ent->curse_dir <= 4)
		forward[ROLL] +=CURSE_MOVEMENT;
	else
		forward[ROLL] -=CURSE_MOVEMENT;
	// don't roll too much
	if ((forward[ROLL] > 0) && (forward[ROLL] > CURSE_MAX_ROLL))
		forward[ROLL] = CURSE_MAX_ROLL;
	else if ((forward[ROLL] < 0) && (forward[ROLL] < -CURSE_MAX_ROLL))
		forward[ROLL] = -CURSE_MAX_ROLL;

	// set view angles 
	for (i = 0 ; i < 3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i]-ent->client->resp.cmd_angles[i]);
	VectorCopy(forward, ent->client->ps.viewangles);
	VectorCopy(forward, ent->client->v_angle);
}

void CurseEffects (edict_t *self, int num, int color)
{
	vec3_t start, up, angle;

	if ((level.framenum % 5) != 0)
		return;
	if (!G_EntIsAlive(self))
		return;

	VectorCopy(self->s.angles, angle);
	angle[ROLL] = GetRandom(0, 20) - 10;
	angle[PITCH] = GetRandom(0, 20) - 10;
	AngleCheck(&angle[ROLL]);
	AngleCheck(&angle[PITCH]);

	AngleVectors(angle, NULL, NULL, up);

	// upside-down minisentry
	if (self->owner && (self->mtype == M_MINISENTRY) 
		&& (self->owner->style == SENTRY_FLIPPED))
		VectorMA(self->s.origin, self->mins[2]-16, up, start);
	else
		VectorMA(self->s.origin, self->maxs[2]+16, up, start);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_LASER_SPARKS);
	gi.WriteByte(num); // number of sparks
	gi.WritePosition(start);
	gi.WriteDir(up);
	gi.WriteByte(color); // 242 = red, 210 = green, 2 = black
	gi.multicast(start, MULTICAST_PVS);
}

