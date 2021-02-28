#include "g_local.h"

void spawnNorm(edict_t *rune, int targ_level, int type);
void spawnCombo(edict_t *rune, int targ_level);
void spawnClassRune(edict_t *rune, int targ_level);
qboolean spawnUnique(edict_t *rune, int index);

//************************************************************************************************
//************************************************************************************************

qboolean V_HiddenMod(edict_t *ent, item_t *rune, imodifier_t *mod)
{
	//Check to see if this is a hidden mod
	if (mod->set == 0)
		return false;

	//Do we have enough set items equipped?
	if (eqSetItems(ent, rune) > mod->set)
		return false;
	else return true;
}

//************************************************************************************************

int eqSetItems(edict_t *ent, item_t *rune)
{
	int i;
	int count = 0;

	//Better safe than sorry
	if (rune->setCode == 0)
		return 0;

	for (i=0; i < 4; ++i)	//loop through only the equip slots
	{
		if ((ent->myskills.items[i].itemtype != TYPE_NONE) && (ent->myskills.items[i].setCode == rune->setCode))
			count++;
	}
	//Return number of matching items
	return count;
}

//************************************************************************************************

int V_GetRuneWeaponPts(edict_t *ent, item_t *rune)
{
	int i, points=0;

	for (i = 0; i < MAX_VRXITEMMODS; ++i)
	{
		//Ignore mods with no mod type
		if (rune->modifiers[i].type == TYPE_NONE)
			continue;

		//Ignore hidden mods if they don't have enough set items equipped
		if (V_HiddenMod(ent, rune, &rune->modifiers[i]))
			continue;

		if (rune->modifiers[i].type & TYPE_WEAPON)
			points += rune->modifiers[i].value;
	}

	if (points < 0)
		return 0;

	return points;
}

int V_GetRuneAbilityPts(edict_t *ent, item_t *rune)
{
	int i, points=0;

	for (i = 0; i < MAX_VRXITEMMODS; ++i)
	{
		//Ignore mods with no mod type
		if (rune->modifiers[i].type == TYPE_NONE)
			continue;

		//Ignore hidden mods if they don't have enough set items equipped
		if (V_HiddenMod(ent, rune, &rune->modifiers[i]))
			continue;

		if (rune->modifiers[i].type & TYPE_ABILITY)
			points += rune->modifiers[i].value;
	}
	
	if (points < 0)
		return 0;

	return points;
}

void V_ApplyRune(edict_t *ent, item_t *rune)
{
	int i;

	if(!rune->itemtype)	//rune equipped? (double checking in some cases)
		return;
	
	for (i = 0; i < MAX_VRXITEMMODS; ++i)
	{
		//Ignore mods with no mod type
		if (rune->modifiers[i].type == TYPE_NONE)
			continue;

		//Ignore hidden mods if they don't have enough set items equipped
		if (V_HiddenMod(ent, rune, &rune->modifiers[i]))
			continue;

		//The only rune mods to worry about are weapon and ability mods
		if (rune->modifiers[i].type & TYPE_WEAPON)
		{
			int weapon, mod;

			//Which weapon mod is this?
			weapon = (rune->modifiers[i].index / 100) - 10;
			mod	= rune->modifiers[i].index % 100;

			//Better safe than sorry
			if ((mod < 0) || (weapon < 0))
				continue;

			if ((mod < MAX_WEAPONMODS) && (weapon < MAX_WEAPONS))
			{
				//Increase the player's current level
				weaponskill_t *wepmod = &(ent->myskills.weapons[weapon].mods[mod]);
				wepmod->current_level += rune->modifiers[i].value;

				//Cap current_level to the hard maximum
				if ((wepmod->hard_max) && (wepmod->current_level > wepmod->hard_max))
					wepmod->current_level = wepmod->hard_max;
			}
		}
		else if (rune->modifiers[i].type & TYPE_ABILITY)
		{
            int ability;
			ability = rune->modifiers[i].index;

			//Better safe than sorry
			if (ability < 0)
				continue;

			if (ability < MAX_ABILITIES)
			{
				//Increase the player's current level
				upgrade_t *abilitymod = &(ent->myskills.abilities[ability]);
				abilitymod->current_level += rune->modifiers[i].value;

				//Cap current_level to the hard maximum (2 * max for abilities)
				if (abilitymod->current_level > abilitymod->hard_max)
					abilitymod->current_level = abilitymod->hard_max;
			}
		}
	} 	
}

//************************************************************************************************
//************************************************************************************************

void adminSpawnRune(edict_t *self, int type, int index)
{
	gitem_t *item;
	edict_t *rune;

	item = FindItem("Rune");		// get the item properties
	rune = Drop_Item(self, item);	// create the entity that holds those properties
	V_ItemClear(&rune->vrxitem);	// initialize the rune
	rune->vrxitem.quantity = 1;
	
	switch(type)
	{
	case ITEM_WEAPON:		spawnNorm(rune, index, ITEM_WEAPON); return;
	case ITEM_ABILITY:		spawnNorm(rune, index, ITEM_ABILITY); return;
	case ITEM_COMBO:		spawnCombo(rune, index); return;
	case ITEM_CLASSRUNE:	spawnClassRune(rune, index); return;
	//Try to spawn a unique (or a random one if it fails to find one at the index)
	case ITEM_UNIQUE:		if (!spawnUnique(rune, index)) spawnNorm(rune, index, 0); return;
	}

	//Randomize id
	strcpy(rune->vrxitem.id, GetRandomString(16));

	//Free the ent (if no rune was spawned)
	G_FreeEdict(rune);
}

//************************************************************************************************
//************************************************************************************************

void V_TossRune (edict_t *rune, float h_vel, float v_vel)
{
	vec3_t forward;

	rune->s.angles[YAW] = GetRandom(0, 360);
	AngleVectors(rune->s.angles, forward, NULL, NULL);
	VectorClear(rune->velocity);
	VectorScale (forward, h_vel, rune->velocity);
	rune->velocity[2] = v_vel;
}

/* az- This function is for avoiding redundant abilities within a rune. */
void fixRuneIndexes(edict_t *rune, int i)
{
	int b_i;
	// do a backward's iteration, if we find the ability then..
	for (b_i = (i - 1); b_i > -1; b_i--)
	{
		// so we already have one of these?
		// add onto it then, discard this one.
		if (rune->vrxitem.modifiers[b_i].index == rune->vrxitem.modifiers[i].index &&
			rune->vrxitem.modifiers[b_i].type == rune->vrxitem.modifiers[i].type)
		{
			if (rune->vrxitem.modifiers[b_i].type == TYPE_ABILITY) // It's an ability?
			{
					// invalidate this modifier.
					rune->vrxitem.modifiers[i].index = 0;
					rune->vrxitem.modifiers[i].type = TYPE_NONE;
					if (GetAbilityUpgradeCost(rune->vrxitem.modifiers[b_i].index) > 1)
					{
						// sum the modifiers.
						rune->vrxitem.modifiers[b_i].value += rune->vrxitem.modifiers[i].value;
					}
					rune->vrxitem.modifiers[i].value = 0;
			}
		}
	}
}


/* az- With this, our rune generation woes are finished, finally! */
abildefinition_t *getRandomAbility();
void V_CreateAbilityModifier(edict_t* rune, qboolean is_class, int i)
{
	abildefinition_t *ability; // Get ability description
	int hard_max;

	if (is_class)
		ability = getClassRuneStat(rune->vrxitem.classNum);
	else
	{
		ability = getRandomAbility();
		while (!ability)
			ability = getRandomAbility();
	}

	if (ability->index < 0 || ability->index > MAX_ABILITIES-1) 
		return; // we can't use this ability, it's invalid due to its index

	// alright so we got a valid ability
	rune->vrxitem.modifiers[i].index = ability->index;

	// Assign it a type
	rune->vrxitem.modifiers[i].type = TYPE_ABILITY;

	if (GetAbilityUpgradeCost(ability->index) > 1) // No runes that have cost 2+ stuff should get over...
	{
		// for class runes, having this kind of abilities that are one-pointed or more is stupid
		if (!ability->start || generalabmode->value || !is_class) 
			rune->vrxitem.modifiers[i].value = 1;
		else 
		{
			// if it has a starting value then we must invalidate this modifier. It's pointless to have it.
			rune->vrxitem.modifiers[i].index = 0;
			rune->vrxitem.modifiers[i].value = 0;
			rune->vrxitem.modifiers[i].type = TYPE_NONE;
		}
	}
	else
	{
		rune->vrxitem.modifiers[i].value = GetRandom(1, RUNE_ABILITY_MAXVALUE);
	}

	hard_max = getHardMax(ability->index, !is_class, 0);
	if (rune->vrxitem.modifiers[i].value > hard_max) // this ability goes way too high? trim it
		rune->vrxitem.modifiers[i].value = hard_max;

	rune->vrxitem.itemLevel += rune->vrxitem.modifiers[i].value;

	fixRuneIndexes(rune, i);
}

edict_t *V_SpawnRune (edict_t *self, edict_t *attacker, float base_drop_chance, float levelmod)
{
	int		iRandom;
	int		targ_level;
	float	temp = 0;
	gitem_t *item;
	edict_t *rune;

	attacker = G_GetClient(attacker);

	if (!attacker || !attacker->client)
		return NULL;

	if(!self->client && self->activator && self->svflags & SVF_MONSTER)
	{
		temp = (float) (self->monsterinfo.level + 1) / (attacker->myskills.level + 1);
		
		if (base_drop_chance * temp < random())
			return NULL;

		//set target level
		targ_level = (self->monsterinfo.level + attacker->myskills.level) / 2;

		// don't allow runes higher than attacker's level
		if (targ_level > attacker->myskills.level)
			targ_level = attacker->myskills.level;
	}
	else if (attacker != self && attacker != world)
	{
		//boss has a greater chance of dropping a rune
		if (attacker->myskills.boss > 0)
            temp = (float)attacker->myskills.boss / 5.0;
		else
			temp = (float) (self->myskills.level + 1) / (attacker->myskills.level + 1);
		
		// miniboss has greater chance of dropping a rune
		if (IsNewbieBasher(self))
			temp *= 2;
		
		//boss has a greater chance of dropping a rune
		temp *= 1.0 + ((float)self->myskills.boss / 2.0);
		
		if (base_drop_chance * temp < random())
			return NULL;
		
		//set target level
		targ_level = (self->myskills.level + attacker->myskills.level) / 2;
	}
	else
		return NULL;

	item = FindItem("Rune");		// get the item properties
	rune = Drop_Item(self, item);	// create the entity that holds those properties
	V_ItemClear(&rune->vrxitem);	// initialize the rune

	if (levelmod)
		targ_level *= levelmod;
	
	rune->vrxitem.quantity = 1;

	//Spawn a random rune
    iRandom = GetRandom(0, 1000);

	if (iRandom < CHANCE_UNIQUE)
	{
		//spawn a unique 
		// vrx chile 1.4 no uniques until someone makes them
		if (!spawnUnique(rune, 0))
			spawnNorm(rune, targ_level, 0);
	}
	else if (iRandom < CHANCE_UNIQUE + CHANCE_CLASS)
	{
		//spawn a class-specific rune
		spawnClassRune(rune, targ_level);
	}
	else if (iRandom < CHANCE_UNIQUE + CHANCE_CLASS + CHANCE_COMBO)
	{
		//spawn a combo rune
		spawnCombo(rune, targ_level);
	}
	else if (iRandom < CHANCE_UNIQUE + CHANCE_CLASS + CHANCE_COMBO + CHANCE_NORM)
	{
		//spawn a normal rune
		spawnNorm(rune, targ_level, 0);
	}
	else
	{
		G_FreeEdict(rune);
		return NULL;
	}

	//Randomize id
	strcpy(rune->vrxitem.id, GetRandomString(16));

	return rune;
}

void SpawnRune (edict_t *self, edict_t *attacker, qboolean debug)
{
	int		iRandom;
	int		targ_level = 0;
	float	temp = 0;
	gitem_t *item;
	edict_t *rune;

	attacker = G_GetClient(attacker);

	if (!attacker)
		return;

	if(!self->client)
	{
		// is this a world monster?
		if (self->mtype && (self->svflags & SVF_MONSTER) && self->activator && !self->activator->client)
		{
			if (IsABoss(self) || (self->mtype == M_COMMANDER))
			//boss has a 100% chance to spawn a rune
				temp = (float) (self->monsterinfo.level + 1) / (attacker->myskills.level + 1) * 100.0;
			else if (self->monsterinfo.bonus_flags & BF_UNIQUE_LIGHTNING 
				|| self->monsterinfo.bonus_flags & BF_UNIQUE_FIRE)
			// unique monsters have a 50% chance to spawn a rune
				temp = (float) (self->monsterinfo.level + 1) / (attacker->myskills.level + 1) * 75.0;
			else if (self->monsterinfo.bonus_flags & BF_CHAMPION)
			// champion monsters have a 15% chance to spawn a rune
				temp = (float) (self->monsterinfo.level + 1) / (attacker->myskills.level + 1) * 15.0; // from 2%
			else
			// monsters have a 5% chance to spawn a rune NOP MONSTERS DON'T DROP RUNES
				//temp = (float) (self->monsterinfo.level + 1) / (attacker->myskills.level + 1) * 5.0; // from 0.2%
				temp = 1; // 1% to drop runes.
			//gi.dprintf("%.3f\n", temp*RUNE_SPAWN_BASE);

			if (RUNE_SPAWN_BASE * temp < random())
				return;

			//set target level
			targ_level = (self->monsterinfo.level + attacker->myskills.level) / 2;
			// don't allow runes higher than attacker's level
			if (targ_level > attacker->myskills.level)
				targ_level = attacker->myskills.level;
		}
		else
			return;
	}
	else if (attacker != self)
	{
		//boss has a greater chance of dropping a rune
		if (attacker->myskills.boss > 0)
            temp = (float)attacker->myskills.boss * 5;
		else
			temp = (float) (self->myskills.level + 1) * 5.0 / (attacker->myskills.level + 1);
		
		// miniboss has greater chance of dropping a rune
		if (IsNewbieBasher(self))
			temp *= 2;
		
		//boss has a greater chance of dropping a rune
		temp *= 1.0 + ((float)self->myskills.boss / 2.0);
		
		if (RUNE_SPAWN_BASE * temp < random())
			return;
		
		//set target level
		targ_level = (self->myskills.level + attacker->myskills.level) / 2;
	}
	else if (debug == false)
		return;

	item = FindItem("Rune");		// get the item properties
	rune = Drop_Item(self, item);	// create the entity that holds those properties
	V_ItemClear(&rune->vrxitem);	// initialize the rune

	//set target level to 20 if we are debugging
	if (debug == true)
		targ_level = 20;
	
	rune->vrxitem.quantity = 1;

	//Spawn a random rune
    iRandom = GetRandom(0, 1000);

	if (iRandom < CHANCE_UNIQUE)
	{
		//spawn a unique
#ifdef ENABLE_UNIQUES
		if (!spawnUnique(rune, 0))
#endif
			spawnNorm(rune, targ_level, 0);
	}
	else if (iRandom < CHANCE_UNIQUE + CHANCE_CLASS)
	{
		//spawn a class-specific rune
		spawnClassRune(rune, targ_level);
	}
	else if (iRandom < CHANCE_UNIQUE + CHANCE_CLASS + CHANCE_COMBO)
	{
		//spawn a combo rune
		spawnCombo(rune, targ_level);
	}
	else if (iRandom < CHANCE_UNIQUE + CHANCE_CLASS + CHANCE_COMBO + CHANCE_NORM)
	{
		//spawn a normal rune
		spawnNorm(rune, targ_level, 0);
	}
	else
	{
		G_FreeEdict(rune);
		return;
	}

	//Randomize id
	strcpy(rune->vrxitem.id, GetRandomString(16));
}

//************************************************************************************************

int GetAbilityUpgradeCost(int index);

void spawnNorm(edict_t *rune, int targ_level, int type)
{
    int x = GetRandom(1, 100);
	int i;
	int max_mods, num_mods;

	//Admins can control the type of rune
	if (type > 0)
	{
		switch(type)
		{
		case ITEM_WEAPON:	x = 100; break;
		case ITEM_ABILITY:	x = 0; break;
		}
	}

	if (x > 50) //weapon rune
	{
		int weaponIndex = GetRandom(0, MAX_WEAPONS-1);	// random weapon

		rune->vrxitem.itemtype = ITEM_WEAPON;

		max_mods = 1 + (0.25 * targ_level); //This means lvl 16+ can get all 5 mods
		if (max_mods > MAX_WEAPONMODS)
			max_mods = MAX_WEAPONMODS;
		num_mods = GetRandom(0, max_mods);

		//set the weapon type (just in case there were 0 mods)
		rune->vrxitem.modifiers[0].index = ((weaponIndex + 10) * 100);
		
		for (i = 0; i < num_mods; ++i)
		{
			int modIndex	= i;

			//25% chance for rune mod not to show up
			if (GetRandom(0, 4) == 0)
				continue;

			rune->vrxitem.modifiers[i].value = GetRandom(1, RUNE_WEAPON_MAXVALUE);
			rune->vrxitem.modifiers[i].index = ((weaponIndex + 10) * 100) + modIndex;	// ex: 1800 = rg damage
			rune->vrxitem.modifiers[i].type = TYPE_WEAPON;

			//Fix 1 time upgrades to 1 pt
			if ((modIndex > 3) || ((modIndex > 2) && (weaponIndex != WEAPON_SWORD)))
			{
				if (rune->vrxitem.modifiers[i].value > 1)
					rune->vrxitem.modifiers[i].value = 1;
			}
			rune->vrxitem.itemLevel += rune->vrxitem.modifiers[i].value;
		}
		rune->vrxitem.numMods = CountRuneMods(&rune->vrxitem);
		rune->s.effects |= EF_PENT;
	}
	else //ability rune
	{
		rune->vrxitem.itemtype = ITEM_ABILITY;

		max_mods = 1 + (0.25 * targ_level); //This means lvl 20+ can get 6 mods
		if (max_mods > MAX_VRXITEMMODS)
			max_mods = MAX_VRXITEMMODS;
		num_mods = GetRandom(0, max_mods);

		for (i=0; i < num_mods; ++i)
		{
			//25% chance for rune mod not to show up
			if (GetRandom(0, 4) == 0)
				continue;
			
			V_CreateAbilityModifier(rune, false, i);
		}
		rune->vrxitem.numMods = CountRuneMods(&rune->vrxitem);
		rune->s.effects |= EF_QUAD;
	}
}

//************************************************************************************************

void spawnClassRune(edict_t *rune, int targ_level)
{
	int i;
	int max_mods, num_mods;

	max_mods = 1 + (0.2 * targ_level);	//This means lvl 15+ can get 4 mods
	if (max_mods > 4)
		max_mods = 4;
	num_mods = GetRandom(1, max_mods); // from 1 - don't be a dick
	rune->vrxitem.itemtype = ITEM_CLASSRUNE;
	rune->vrxitem.classNum = GetRandom(1, CLASS_MAX);	//class number

	if (rune->vrxitem.classNum == CLASS_WEAPONMASTER) // These ain't got class runes.
	{
		G_FreeEdict(rune);
		return;
	}

	for (i = 0; i < num_mods; ++i)
	{
		V_CreateAbilityModifier(rune, true, i);
	}
	rune->vrxitem.numMods = CountRuneMods(&rune->vrxitem);
	rune->s.effects |= EF_COLOR_SHELL;
	rune->s.renderfx |= RF_SHELL_CYAN;
}

//************************************************************************************************
//************************************************************************************************

char *UniqueParseString(char **iterator)
{
	char buf[30];
	int i;

	for (i = 0; i < strlen(*iterator); ++i)
	{
		if (*(*iterator + i) != ',')
            buf[i] = *(*iterator + i);
		else break;
	}
	buf[i] = 0;
    *iterator += i+1;
	return va("%s", buf);
}

//************************************************************************************************

int UniqueParseInteger(char **iterator)
{
	char buf[30];
	int i;

	for (i = 0; i < strlen(*iterator); ++i)
	{
		if (*(*iterator + i) != ',')
            buf[i] = *(*iterator + i);
		else break;
	}
	buf[i] = 0;
    *iterator += i+1;
	return atoi(buf);
}

//************************************************************************************************

qboolean spawnUnique(edict_t *rune, int index)
{
	int j, i;
	char filename[256];
	char buf[256];
	FILE *fptr;
	j = i = 0;

	//determine path
	#if defined(_WIN32) || defined(WIN32)
		sprintf(filename, "%s\\%s", game_path->string, "settings\\Uniques\\uniques.csv");
	#else
		sprintf(filename, "%s/%s", game_path->string, "settings/Uniques/uniques.csv");
	#endif

	if ((fptr = fopen(filename, "r")) != NULL)
	{
		int linenumber, maxlines;
		char *iterator;
		long size;
		int count = 0;

		//Determine file size
		fseek (fptr, 0, SEEK_END);
		size=ftell (fptr);
		rewind(fptr);

		//Find a unique
		maxlines = V_tFileCountLines(fptr, size);

		if ((index == 0) || (index > maxlines))
		{
			if (maxlines > 1)
				linenumber = GetRandom(1, maxlines-1);
			else return false;
		}
		else linenumber = index;

		V_tFileGotoLine(fptr, linenumber, size);
		fgets(buf, 256, fptr);

		//Load the rune stats
		iterator = buf;
		rune->vrxitem.itemtype = UniqueParseInteger(&iterator);
		rune->vrxitem.untradeable = UniqueParseInteger(&iterator);
		strcpy(rune->vrxitem.name, UniqueParseString(&iterator));
		rune->vrxitem.numMods = UniqueParseInteger(&iterator);
		rune->vrxitem.setCode = UniqueParseInteger(&iterator);

		//Load the mods
		for(i = 0; i < 6; ++i)
		{
			rune->vrxitem.modifiers[i].type = UniqueParseInteger(&iterator);
			rune->vrxitem.modifiers[i].index = UniqueParseInteger(&iterator);
			rune->vrxitem.modifiers[i].value = UniqueParseInteger(&iterator);
			rune->vrxitem.modifiers[i].set = UniqueParseInteger(&iterator);
			rune->vrxitem.itemLevel += rune->vrxitem.modifiers[i].value;
		}

		rune->s.effects |= EF_COLOR_SHELL;

		if (rune->vrxitem.setCode != 0) rune->s.renderfx |= RF_SHELL_GREEN;
		else rune->s.renderfx |= RF_SHELL_YELLOW;

		fclose (fptr);
		return true;
	}
	else 
	{
		G_FreeEdict(rune);
		return false;
	}
}

//************************************************************************************************
//************************************************************************************************

void spawnCombo(edict_t *rune, int targ_level)
{
	int i;
	int max_mods, num_mods;

	rune->vrxitem.itemtype = ITEM_COMBO;
	max_mods = 1 + (0.2 * targ_level);
	if (max_mods > 4)
		max_mods = 4;
	num_mods = GetRandom(0, max_mods);
	
	for (i = 0; i < num_mods; ++i)
	{
		int type = GetRandom(1, 10);
		//50% chance of ability mod
		if (type > 5)
		{
			V_CreateAbilityModifier(rune, false, i);
		}
		else	//50% chance of weapon mod
		{
			int modIndex	= GetRandom(0, MAX_WEAPONMODS-1);
			int wIndex		= GetRandom(0, MAX_WEAPONS-1);

			rune->vrxitem.modifiers[i].value = GetRandom(1, 3);
			rune->vrxitem.modifiers[i].index = ((wIndex + 10) * 100) + modIndex;	// ex: 1800 = rg damage
			rune->vrxitem.modifiers[i].type = TYPE_WEAPON;

			//Fix 1 time upgrades to 1 pt
			if ((modIndex > 3) || ((modIndex > 2) && (wIndex != WEAPON_SWORD)))
			{
				if (rune->vrxitem.modifiers[i].value > 1)
					rune->vrxitem.modifiers[i].value = 1;
			}
			rune->vrxitem.itemLevel += rune->vrxitem.modifiers[i].value;
		}
	}
	rune->vrxitem.numMods = CountRuneMods(&rune->vrxitem);
	rune->s.effects |= EF_COLOR_SHELL;
	rune->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE;
}

//************************************************************************************************
//************************************************************************************************

void PurchaseRandomRune(edict_t *ent, int runetype)
{
	int	cost;
	edict_t *rune;
	item_t *slot;
	char buf[64];

	cost = RUNE_COST_BASE + RUNE_COST_ADDON * ent->myskills.level;
	if (ent->myskills.credits < cost)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need %d credits to purchase a rune.\n", cost);
		return;
	}

	slot = V_FindFreeItemSlot(ent);
	if (!slot)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough inventory space!\n");
		return;
	}

	// rune pick-up delay
	if (ent->client->rune_delay > level.time)
		return;

	rune = G_Spawn();				// create a rune
	V_ItemClear(&rune->vrxitem);	// initialize the rune

	ent->myskills.credits -= cost;
	
	if (runetype)
	{
		spawnNorm(rune, ent->myskills.level, runetype);
	}
	else if (random() > 0.5)
	{
		spawnNorm(rune, ent->myskills.level, ITEM_WEAPON);
	}
	else
	{
		spawnNorm(rune, ent->myskills.level, ITEM_ABILITY);
	}
    
	if (Pickup_Rune(rune, ent) == false)
	{
		G_FreeEdict(rune);
		//gi.dprintf("WARNING: PurchaseRandomRune() was unable to spawn a rune\n");
		return;
	}
	G_FreeEdict(rune);

	//Find out what the player bought
	strcpy(buf, GetRuneValString(slot));
	switch(slot->itemtype)
	{
	case ITEM_WEAPON:	strcat(buf, va(" weapon rune (%d mods)", slot->numMods));	break;
	case ITEM_ABILITY:	strcat(buf, va(" ability rune (%d mods)", slot->numMods));	break;
	case ITEM_COMBO:	strcat(buf, va(" combo rune (%d mods)", slot->numMods));	break;
	}

	//send the message to the player
	safe_cprintf(ent, PRINT_HIGH, "You bought a %s.\n", buf);
	safe_cprintf(ent, PRINT_HIGH, "You now have %d credits left. \n", ent->myskills.credits);
	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/gold.wav"), 1, ATTN_NORM, 0);

	//Save the player
	if (savemethod->value == 1)
		SaveCharacter(ent);
	else if (savemethod->value == 0)
	{
		char path[MAX_QPATH];
		memset(path, 0, MAX_QPATH);
		VRXGetPath(path, ent);
		VSF_SaveRunes(ent, path);
	}else if (savemethod->value == 3)
	{
		VSFU_SaveRunes(ent);
	}

	//write to the log
	gi.dprintf("INFO: %s purchased a level %d rune (%s).\n", 
		ent->client->pers.netname, slot->itemLevel, slot->id);
	WriteToLogfile(ent, va("Purchased a level %d rune (%s) for %d credits. Player has %d credits left.\n",
		slot->itemLevel, slot->id, cost, ent->myskills.credits));
}

//************************************************************************************************
//************************************************************************************************

qboolean Pickup_Rune (edict_t *ent, edict_t *other)
{
	item_t *slot;

	if (!other->client)
		return false;

	//Show the user what kind of rune it is
	if (level.time > other->msg_time)
	{
		V_PrintItemProperties(other, &ent->vrxitem);
		other->msg_time = level.time+1;
	}

	if (other->client->rune_delay > level.time)
		return false;

	//4.2 if it's worthless, then just destroy it
	if (ent->vrxitem.itemLevel < 1)
	{
		G_FreeEdict(ent);
		return false;
	}

	//Can we pick it up?
	if (!V_CanPickUpItem(other))
		return false;


	//Give them the rune
	slot = V_FindFreeItemSlot(other);
	V_ItemCopy(&ent->vrxitem, slot);

	//Save the player file (prevents lost runes)
	if (savemethod->value == 1)
		SaveCharacter(other);
	else if (savemethod->value == 0)
	{
		char path[MAX_QPATH];
		memset(path, 0, MAX_QPATH);
		VRXGetPath(path, other);
		VSF_SaveRunes(other, path);
	}

	other->client->rune_delay = level.time + RUNE_PICKUP_DELAY;

	//Done
	return true;
}

//************************************************************************************************
//************************************************************************************************

void V_ItemCopy(item_t *source, item_t *dest)
{
	memcpy(dest, source, sizeof(item_t));
}

//************************************************************************************************

void V_ItemClear(item_t *item)
{
	memset(item, 0, sizeof(item_t));
}

//************************************************************************************************

item_t *V_FindFreeItemSlot (edict_t *ent)
{
	int i;

    //Fill items backwards from the bottom of the stash
	for (i = MAX_VRXITEMS-1; i > 2; --i)
	{
		if (ent->myskills.items[i].itemtype)
			continue;
		return &ent->myskills.items[i];
	}
	return NULL;
}

//************************************************************************************************

item_t *V_FindFreeTradeSlot(edict_t *ent, int index)
{
	int i;
	int count = 0;

    //Check items backwards from the bottom of the stash
	for (i = MAX_VRXITEMS-1; i > 2; --i)
	{
		if (ent->myskills.items[i].itemtype)
			continue;
		++count;
		if (count == index)
			return &ent->myskills.items[i];
	}
	return NULL;
}

//************************************************************************************************

qboolean V_CanPickUpItem (edict_t *ent)
{
	int i;

	// only allow clients that are alive and haven't just
	// respawned to pick up a rune
	//if (!ent->client || !G_EntIsAlive(ent) || (ent->client->respawn_time > level.time))
	//	return false;
	if (!ent || !ent->inuse || !ent->client || G_IsSpectator(ent)
		|| (ent->client->respawn_time > level.time))
		return false;
	// do we have any space in our inventory?
	// Skip hand, neck, and belt slots           intentando equipar en HEAD
	for (i=4; i < MAX_VRXITEMS; ++i)
	{
		if (!ent->myskills.items[i].itemtype)
			return true;
	}
	return false;
}

//************************************************************************************************

void V_PrintItemProperties(edict_t *player, item_t *item)
{
	char buf[256];
	int i;

	//Did they find a unique?
	if (strlen(item->name) > 0)
	{
		gi.centerprintf(player, "WOW! It's \"%s\"!!\n", item->name);
		return;
	}

	strcpy(buf, GetRuneValString(item));
	
	switch(item->itemtype)
	{
	case ITEM_WEAPON:	strcat(buf, " weapon rune ");	break;
	case ITEM_ABILITY:	strcat(buf, " ability rune ");	break;
	case ITEM_COMBO:	strcat(buf, " combo rune ");	break;
	case ITEM_CLASSRUNE:strcpy(buf, va(" %s rune ", GetRuneValString(item)));	break;
	}

	if(item->numMods == 1) strcat(buf, "(1 mod)");
	else strcat(buf, va("(%d mods)", item->numMods));

	for (i = 0; i < MAX_VRXITEMMODS; ++i)
	{
		char temp[32];

		//skip bad mod types
		if ((item->modifiers[i].type != TYPE_ABILITY) && (item->modifiers[i].type != TYPE_WEAPON) || item->modifiers[i].value < 1)
			continue;

		switch(item->modifiers[i].type)
		{
		case TYPE_ABILITY:	strcpy(temp, va("%s:", GetAbilityString(item->modifiers[i].index)));				break;
		case TYPE_WEAPON:	strcpy(temp, va("%s %s:", 
								GetShortWeaponString((item->modifiers[i].index / 100)-10), 
								GetModString((item->modifiers[i].index / 100)-10, item->modifiers[i].index % 100)));	break;
		}
		padRight(temp, 20);
		strcat(buf, va("\n    %s(%d)", temp, item->modifiers[i].value));
	}
	
	gi.centerprintf(player, "%s\n", buf);
}

//************************************************************************************************

void V_ItemSwap(item_t *item1, item_t *item2)
{
	item_t temp;
	memcpy(&temp, item1, sizeof(item_t));
	memcpy(item1, item2, sizeof(item_t));
	memcpy(item2, &temp, sizeof(item_t));
	//memcpy(item3, &temp, sizeof(item_t)); // runa head 
}

//************************************************************************************************

void V_ResetAllStats(edict_t *ent)
{
	int i;

	//Reset abilities:
	for (i = 0; i < MAX_ABILITIES; ++i)
	{
		//if a rune gave the player a bonus beyond their normal skillset
		if (ent->myskills.abilities[i].runed == true)
		{
			//remove that bonus to their skillset
			ent->myskills.abilities[i].disable = true;
			ent->myskills.abilities[i].runed = false;
			ent->myskills.abilities[i].hidden = false;
		}
		ent->myskills.abilities[i].current_level = ent->myskills.abilities[i].level;
	}

	//Reset the player's weapon stats
	resetWeapons(ent);
}

//************************************************************************************************

void V_EquipItem(edict_t *ent, int index)
{
	int i, wpts, apts, total_pts, clvl = ent->myskills.level;

	// calculate number of weapon and ability points separately
	wpts = V_GetRuneWeaponPts(ent, &ent->myskills.items[index]);
	apts = V_GetRuneAbilityPts(ent, &ent->myskills.items[index]);
	// calculate weighted total
	total_pts = ceil(0.5*wpts + 0.75*apts);//was 0.66,2.0
	//gi.dprintf("wpts = %d, apts = %d, total = %d\n", wpts, apts, total_pts);

	if(index < 4) // cambiar de 3 a 4 
	{
		//remove an item
		item_t *slot = V_FindFreeItemSlot(ent);
		if (slot == NULL)
		{
			safe_cprintf(ent, PRINT_HIGH, "Not enough room in your stash.\n");
			return;
		}
		V_ItemSwap(&ent->myskills.items[index], slot);
		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/boots.wav"), 1, ATTN_NORM, 0);
		safe_cprintf(ent, PRINT_HIGH, "Item successfully placed in your stash.\n");
	}
	//Everyone but admins have a minimum level requirement to use a rune. (easier for rune testing)
	// vrxchile 2.0: only 999 admins are able to test runes.
	else if ((ent->myskills.administrator < 999) && (ent->myskills.level < total_pts))
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to be level %d to use this rune.\n", total_pts);
		return;
	}
	else if (index < MAX_VRXITEMS)
	{
		int type = ent->myskills.items[index].itemtype;

		if (type & ITEM_UNIQUE)
			type ^= ITEM_UNIQUE;

		//equip an item
		switch(type)
		{
		case ITEM_WEAPON:
			V_ItemSwap(&ent->myskills.items[index], &ent->myskills.items[0]); //put on hand slot
			if (eqSetItems(ent, &ent->myskills.items[0]) == 3)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/blessedaim.wav"), 1, ATTN_NORM, 0);
			else gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/glovesmetal.wav"), 1, ATTN_NORM, 0);
			break;
		case ITEM_ABILITY:
			V_ItemSwap(&ent->myskills.items[index], &ent->myskills.items[1]); //put on neck slot
			if (eqSetItems(ent, &ent->myskills.items[0]) == 3)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/blessedaim.wav"), 1, ATTN_NORM, 0);
			else gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/amulet.wav"), 1, ATTN_NORM, 0);
			break;
		case ITEM_COMBO:
			V_ItemSwap(&ent->myskills.items[index], &ent->myskills.items[2]); //put on belt slot
			if (eqSetItems(ent, &ent->myskills.items[0]) == 3)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/blessedaim.wav"), 1, ATTN_NORM, 0);
			else gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/belt.wav"), 1, ATTN_NORM, 0);
			break;
		case ITEM_CLASSRUNE:
			V_ItemSwap(&ent->myskills.items[index], &ent->myskills.items[3]); //put on head slot
			if (eqSetItems(ent, &ent->myskills.items[0]) == 3)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/blessedaim.wav"), 1, ATTN_NORM, 0);
			else gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/amulet.wav"), 1, ATTN_NORM, 0);
			break;
		}
		safe_cprintf(ent, PRINT_HIGH, "Item successfully equipped.\n");
	}

	//Reset all rune info
	V_ResetAllStats(ent);
	for (i = 0; i < 4; ++i)
	{
		if (ent->myskills.items[i].itemtype != TYPE_NONE)
			V_ApplyRune(ent, &ent->myskills.items[i]);
	}
}

//************************************************************************************************

void cmd_Drink(edict_t *ent, int itemtype, int index)
{
	int i;
	item_t *slot = NULL;
	qboolean found = false;

	//Don't drink too many potions too quickly
	if (ent->client->ability_delay > level.time)
		return;

	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
		return;

	if (index)
	{
		slot = &ent->myskills.items[index-1];
		found = true;
	}
	else
	{
		//Find item in inventory
		for (i = 4; i < MAX_VRXITEMS; ++i)
		{
			if (ent->myskills.items[i].itemtype == itemtype)
			{
				slot = &ent->myskills.items[i];
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have none in stock.\n");
		return;
	}

	//use it
	switch(itemtype)
	{
	case ITEM_POTION:
		{
			int max_hp = MAX_HEALTH(ent);

			if (ent->health < max_hp)
			{
				//Use the potion
				ent->health += max_hp/* / 3*/; // make them useful once again vrx chile 1.4
				if (ent->health > max_hp)
					ent->health = max_hp;

				//You can only drink 1/sec
				ent->client->ability_delay = level.time + 3.3;
				
				//Play sound
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/potiondrink.wav"), 1, ATTN_NORM, 0);
				
				//Consume a potion
				slot->quantity--;
				safe_cprintf(ent, PRINT_HIGH, "You drank a potion. Potions left: %d\n", slot->quantity);
				if (slot->quantity < 1)
					V_ItemClear(slot);
			}
			else safe_cprintf(ent, PRINT_HIGH, "You don't need to drink this yet, you have lots of health.\n");
			return;

		}
		break;
	case ITEM_ANTIDOTE:
		{
			if (que_typeexists(ent->curses, 0))
			{
				int i;
				//Use the potion
				for (i = 0; i < QUE_MAXSIZE; ++i)
				{
					que_t *curse = &ent->curses[i];
					if ((curse->ent && curse->ent->inuse) && (curse->ent->atype != HEALING && curse->ent->atype != BLESS))
					{
						//destroy the curse
						if (curse->ent->enemy && (curse->ent->enemy == ent))
							G_FreeEdict(curse->ent);
						// remove entry from the queue
						curse->time = 0;
						curse->ent = NULL;
					}
				}

				//Give them a short period of curse immunity
				ent->holywaterProtection = level.time + 5.0; //5 seconds immunity

				//You can only drink 1/sec
				ent->client->ability_delay = level.time + 1.0;
				
				//Play sound
				gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/rare.wav"), 1, ATTN_NORM, 0);
				
				//Consume a potion
				slot->quantity--;
				safe_cprintf(ent, PRINT_HIGH, "You used some holywater. Vials left: %d\n", slot->quantity);
				if (slot->quantity < 1)
					V_ItemClear(slot);
				
			}
			else safe_cprintf(ent, PRINT_HIGH, "You are not cursed, so you don't need to use this yet.\n");
			return;
		}
		break;
	}
}

//************************************************************************************************
// Parameters:
	// selected - preceding letter used during trading. It is usualy a space (' ')
	// item		- item to get the string from
//************************************************************************************************
char *V_MenuItemString(item_t *item, char selected)
{
	//If item has a name, use that instead
	if (strlen(item->name) > 0)
		return va("%c%s", selected, item->name);
	else
	{
		//Print item, depending on the item type
		switch(item->itemtype)
		{
		case ITEM_NONE:			return " <Empty>";
		case ITEM_WEAPON:		return va("%cWeapon rune  (%d)", selected, item->itemLevel);
		case ITEM_ABILITY:		return va("%cAbility rune (%d)", selected, item->itemLevel);
		case ITEM_COMBO:		return va("%cCombo rune   (%d)", selected, item->itemLevel);
		case ITEM_CLASSRUNE:	return va("%c%s rune", selected, GetRuneValString(item));
		case ITEM_POTION:		return va("%c%d health potions", selected, item->quantity);
		case ITEM_ANTIDOTE:		return va("%c%d vials of holy water", selected, item->quantity);
		case ITEM_GRAVBOOTS:	return va("%cAnti-Gravity Boots", selected);
		case ITEM_FIRE_RESIST:	return va("%cFire resistant clothing", selected);
		case ITEM_AUTO_TBALL:	return va("%cAuto-Tball", selected);
		default:				return va("%c<Unknown item type>", selected, item->itemLevel);
		}
	}
}

//************************************************************************************************