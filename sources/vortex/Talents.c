#include "g_local.h"

//Gives the player a new talent.
void addTalent(edict_t *ent, int talentID, int maxLevel)
{
	int nextEmptySlot = ent->myskills.talents.count;
	int i = 0;

	//Don't add too many talents.
	if(!(nextEmptySlot < MAX_TALENTS))
		return;

	//Don't add a talent more than once.
	for(i = 0; i < nextEmptySlot; ++i)
		if(ent->myskills.talents.talent[nextEmptySlot].id == talentID)
			return;

	ent->myskills.talents.talent[nextEmptySlot].id = talentID;
	ent->myskills.talents.talent[nextEmptySlot].maxLevel = maxLevel;
	ent->myskills.talents.talent[nextEmptySlot].upgradeLevel = 0;		//Just in case it's not already zero.

	ent->myskills.talents.count++;
}

//Adds all the required talents for said class.
void setTalents(edict_t *ent)
{
	switch(ent->myskills.class_num)
	{
		case CLASS_SOLDIER:
			addTalent(ent, TALENT_IMP_STRENGTH, 5);
			//addTalent(ent, TALENT_IMP_RESIST, 5);
			addTalent(ent, TALENT_BLOOD_OF_ARES, 5); //cambio max a 10 daño aumentado en pvp, invasion y pvm
			addTalent(ent, TALENT_BASIC_HA, 1);
			addTalent(ent, TALENT_MONSTER_MASTERY, 5);
			addTalent(ent, TALENT_BOMBARDIER, 5);
			//addTalent(ent, TALENT_MARTYR, 5);
			addTalent(ent, TALENT_BLAST_RESIST, 5);
			addTalent(ent, TALENT_MAGMINESELF, 1);
			addTalent(ent, TALENT_INSTANTPROXYS, 2);
			return;
		case CLASS_POLTERGEIST:  //This is Alien
		
			addTalent(ent, TALENT_MONSTER_MASTERY, 5);
			addTalent(ent, TALENT_SWARMING, 5);
			addTalent(ent, TALENT_LIFE_REG, 1); //change to life
			addTalent(ent, TALENT_PHANTOM_OBSTACLE, 5);
			addTalent(ent, TALENT_SUPER_HEALER, 5);
			//addTalent(ent, TALENT_SUPERIORITY, 5);
			//addTalent(ent, TALENT_RETALIATION, 5);
			//addTalent(ent, TALENT_PACK_ANIMAL, 5);
			//addTalent(ent, TALENT_MORPHING, 5);
			//addTalent(ent, TALENT_MORE_AMMO, 5);
			//addTalent(ent, TALENT_PHANTOM_COCOON, 5);
			//addTalent(ent, TALENT_EXPLODING_BODIES, 5);
			return;
		case CLASS_DEMON:
			addTalent(ent, TALENT_IMP_CLOAK, 4);
			addTalent(ent, TALENT_ARMOR_VAMP, 3);
			addTalent(ent, TALENT_SECOND_CHANCE, 4);
			addTalent(ent, TALENT_MONSTER_MASTERY, 5);
			addTalent(ent, TALENT_IMP_MINDABSORB, 4);
			addTalent(ent, TALENT_CANNIBALISM, 5);
			addTalent(ent, TALENT_FATAL_WOUND, 5);
			addTalent(ent, TALENT_CHEAPER_CURSES, 5);
			//addTalent(ent, TALENT_CORPULENCE, 5);
			addTalent(ent, TALENT_LIFE_TAP, 5);
			addTalent(ent, TALENT_DIM_VISION, 5);
			addTalent(ent, TALENT_EVIL_CURSE, 5);
			addTalent(ent, TALENT_FLIGHT, 5);
			return;
		case CLASS_ARCANIST:
			addTalent(ent, TALENT_ICE_BOLT, 5);
			addTalent(ent, TALENT_FROST_NOVA, 5);
			addTalent(ent, TALENT_IMP_MAGICBOLT, 5);
			addTalent(ent, TALENT_MANASHIELD, 5);
			addTalent(ent, TALENT_MONSTER_MASTERY, 5);
			addTalent(ent, TALENT_MEDITATION, 5);
			addTalent(ent, TALENT_OVERLOAD, 5);
			addTalent(ent, TALENT_ICE, 4);
			addTalent(ent, TALENT_WIND, 4);
			addTalent(ent, TALENT_STONE, 4);
			addTalent(ent, TALENT_SHADOW, 4);
			addTalent(ent, TALENT_PEACE, 4);
			addTalent(ent, TALENT_TOTEM, 6);
			addTalent(ent, TALENT_VOLCANIC, 5);
			return;
		case CLASS_ENGINEER:
		
			addTalent(ent, TALENT_LASER_PLATFORM, 5);
            addTalent(ent, TALENT_ALARM, 5);
			addTalent(ent, TALENT_ARMOR_REG, 1);
			//addTalent(ent, TALENT_DEFENSIVE_CRATE, 1);
			//addtalent(ent, TALENT_DEFENSIVE_CRATE, 1);
			//addTalent(ent, TALENT_RAPID_ASSEMBLY, 5);
			addTalent(ent, TALENT_PRECISION_TUNING, 5);
			addTalent(ent, TALENT_MONSTER_MASTERY, 5);
			addTalent(ent, TALENT_STORAGE_UPGRADE, 5);
			//addTalent(ent, TALENT_EXTRA_MINISENTRY, 1);
			return;
		case CLASS_PALADIN:
			addTalent(ent, TALENT_MONSTER_MASTERY, 5);
			addTalent(ent, TALENT_BALANCESPIRIT, 5);
			addTalent(ent, TALENT_HOLY_GROUND, 5);
			addTalent(ent, TALENT_UNHOLY_GROUND, 5);
			addTalent(ent, TALENT_BOOMERANG, 5);
			addTalent(ent, TALENT_PURGE, 5);
			addTalent(ent, TALENT_REPEL, 5);
			//addTalent(ent, TALENT_MAG_BOOTS, 5);
			//addTalent(ent, TALENT_LEAP_ATTACK, 5);
			//addTalent(ent, TALENT_MOBILITY, 5);
			addTalent(ent, TALENT_DURABILITY, 5);
			return;
		case CLASS_WEAPONMASTER:
			addTalent(ent, TALENT_BASIC_AMMO_REGEN, 5);
			addTalent(ent, TALENT_COMBAT_EXP, 5);
			addTalent(ent, TALENT_TACTICS, 3);
			addTalent(ent, TALENT_SIDEARMS, 3);
			return;
	default: return;
	}
}

//Erases all talent information.
void eraseTalents(edict_t *ent)
{
	memset(&ent->myskills.talents, 0, sizeof(talentlist_t));
}

//Returns the talent slot with matching talentID.
//Returns -1 if there is no matching talent.
int getTalentSlot(edict_t *ent, int talentID)
{
	int i;
	int num;

	//Make sure the ent is valid
	if(!ent)
	{
		WriteServerMsg(va("getTalentSlot() called with a NULL entity. talentID = %d", talentID), "CRITICAL ERROR", true, true);
		return -1;
	}

	//Make sure we are a player
	if(!ent->client)
	{
		//gi.dprintf(va("WARNING: getTalentSlot() called with a non-player entity! talentID = %d\n", talentID));
		return -1;
	}

	num = ent->myskills.talents.count;

	if (num < 5)
		num = 5;

	for(i = 0; i < ent->myskills.talents.count; ++i)
	{
		if(ent->myskills.talents.talent[i].id == talentID)
			return i;
	}
	return -1;
}

//Returns the talent upgrade level matching talentID.
//Returns -1 if there is no matching talent.
int getTalentLevel(edict_t *ent, int talentID)
{
	int slot = getTalentSlot(ent, talentID);
	
	if(slot < 0)
	{ 
		if (!ent->client) // so it's a morphed player?
			if (ent->owner && ent->owner->inuse && ent->owner->client)
			{
				slot = getTalentSlot(ent->owner, talentID);
				ent = ent->owner;
			}else if (ent->activator && ent->activator->inuse && ent->activator->client)
			{
				slot = getTalentSlot(ent->activator, talentID);
				ent = ent->activator;
			}

			if(slot < 0) // still doesn't exist? k
				return 0; 
	} //;//-1;
	

	return ent->myskills.talents.talent[slot].upgradeLevel;	
}

//Upgrades the talent with a matching talentID
void upgradeTalent(edict_t *ent, int talentID)
{
	int slot = getTalentSlot(ent, talentID);
	talent_t *talent;

	if(slot == -1)
		return;

	talent = &ent->myskills.talents.talent[slot];

	// check for conflicting talents
	if (talentID == TALENT_RAPID_ASSEMBLY && getTalentLevel(ent, TALENT_PRECISION_TUNING) > 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Rapid Assembly can't be combined with Precision Tuning.\n");
		return;
	}
	if (talentID == TALENT_PRECISION_TUNING && getTalentLevel(ent, TALENT_RAPID_ASSEMBLY) > 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Precision Tuning can't be combined with Rapid Assembly.\n");
		return;
	}
	if (talentID == TALENT_CORPULENCE && getTalentLevel(ent, TALENT_LIFE_TAP) > 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Corpulence can't be combined with Life Tap.\n");
		return;
	}
	if (talentID == TALENT_LIFE_TAP && getTalentLevel(ent, TALENT_CORPULENCE) > 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Life Tap can't be combined with Corpulence.\n");
		return;
	}

	if(talent->upgradeLevel == talent->maxLevel)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
		return;
	}
	if(ent->myskills.talents.talentPoints < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
		return;
	}

    //We can upgrade.
	talent->upgradeLevel++;
	ent->myskills.talents.talentPoints--;
	safe_cprintf(ent, PRINT_HIGH, va("%s upgraded to level %d/%d.\n", GetTalentString(talent->id), talent->upgradeLevel, talent->maxLevel));
	safe_cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	//savePlayer(ent);
}

//****************************************
//************* Talent Menus *************
//****************************************

void TalentUpgradeMenu_handler(edict_t *ent, int option)
{
	//Not upgrading
	if(option > 0)
	{
		OpenTalentUpgradeMenu(ent, getTalentSlot(ent, option-1)+1);
	}
	else	//upgrading
	{
		upgradeTalent(ent, (option * -1)-1);
	}
}

//Adds menu lines that describe the general use of the talent.
int writeTalentDescription(edict_t *ent, int talentID)
{
	switch(talentID)
	{
	//Soldier talents
	case TALENT_IMP_STRENGTH:
		addlinetomenu(ent, "Increases damage,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "And resist.", MENU_WHITE_CENTERED);
		
		return 2;
	//case TALENT_IMP_RESIST:
		//addlinetomenu(ent, "Increases resist,", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "but reduces damage.", MENU_WHITE_CENTERED);
		//return 2;
	case TALENT_BLOOD_OF_ARES:
		addlinetomenu(ent, "Increases the damage you", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "give/take as you spree.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Aditional effect in PvM or Invasion", MENU_WHITE_CENTERED);
		return 2;

	case TALENT_MONSTER_MASTERY:
		addlinetomenu(ent, "Damage increased in", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "PvM and Invasion.", MENU_WHITE_CENTERED);
		return 2;

	case TALENT_BASIC_HA:
		addlinetomenu(ent, "Increases ammo pickups.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_BOMBARDIER:
		addlinetomenu(ent, "Reduces self-inflicted", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "grenade damage and", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "reduces cost.", MENU_WHITE_CENTERED);
		return 3;



	//Poltergeist talents
    case TALENT_MORPHING:
		addlinetomenu(ent, "Reduces the cost", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "of your morphs.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MORE_AMMO:
		addlinetomenu(ent, "Increases maximum ammo", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "capacity for", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "tank/caco/flyer/medic.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_SUPERIORITY:
		addlinetomenu(ent, "Increased damage and", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "resistance to monsters.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_RETALIATION:
		addlinetomenu(ent, "Damage increases as", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "health is reduced.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_PACK_ANIMAL:
	    addlinetomenu(ent, "Increased damage and", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "resistance when near", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "friendly morphed", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "players.", MENU_WHITE_CENTERED);
		return 4;

	//Vampire talents
	//case TALENT_IMP_CLOAK:
		//addlinetomenu(ent, "Move while cloaked", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "(must be crouching).", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "1/3 pc cost at night!", MENU_WHITE_CENTERED);
		//return 3;

	case TALENT_ARMOR_VAMP:
		addlinetomenu(ent, "Also gain armor using", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "your vampire skill.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_FATAL_WOUND:
		addlinetomenu(ent, "Adds chance for flesh", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "eater to cause a fatal", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "wound.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_SECOND_CHANCE:
		addlinetomenu(ent, "100% chance of ghost", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "working when hit.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_IMP_MINDABSORB:
		addlinetomenu(ent, "Increases frequency of", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "mind absorb attacks.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_CANNIBALISM:
		addlinetomenu(ent, "Increases your maximum", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "health using corpse eater.", MENU_WHITE_CENTERED);
		return 2;

	//Mage talents
	case TALENT_ICE_BOLT:
		addlinetomenu(ent, "Use 'icebolt' instead of", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "fireball to chill targets.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MEDITATION:
		addlinetomenu(ent, "Recharge your power", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "cubes at a whim (cmd '+manacharge').", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_OVERLOAD:
		addlinetomenu(ent, "Use extra power cubes", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to overload spells,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "increasing their", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "effectiveness", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'overload').", MENU_WHITE_CENTERED);
		return 5;
	case TALENT_FROST_NOVA:
		addlinetomenu(ent, "Special nova spell", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "that chills players.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd frostnova)", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_IMP_MAGICBOLT:
		addlinetomenu(ent, "Power cubes are refunded", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "on successful hits.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MANASHIELD:
		addlinetomenu(ent, "Reduces physical damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "by 80%%. All damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "absorbed consumes power", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "cubes. (cmd manashield)", MENU_WHITE_CENTERED);
		return 4;

	//Engineer talents
	

	//case TALENT_DEFENSIVE_CRATE:
		//addlinetomenu(ent, "Create a Box to absorbe damage", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "(cmd 'box').", MENU_WHITE_CENTERED);
		//return 2;


	case TALENT_LASER_PLATFORM:
		addlinetomenu(ent, "Create a laser platform", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'laserplatform').", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_ALARM:
		addlinetomenu(ent, "Use 'lasertrap' instead", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "of detector to build a", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "laser trap.", MENU_WHITE_CENTERED);
		return 3;

	case TALENT_ARMOR_REG:
		addlinetomenu(ent, "Allow you to regenerate.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "	some armor 	", MENU_WHITE_CENTERED);

		return 3;
	case TALENT_PRECISION_TUNING:
		addlinetomenu(ent, " Increased Damage and ", MENU_WHITE_CENTERED);
		addlinetomenu(ent, " Reduce build time ", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "  Of your devices. ", MENU_WHITE_CENTERED);

		
		return 3;
	case TALENT_STORAGE_UPGRADE:
		addlinetomenu(ent, "Increases ammunition", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "capacity of SS/sentry/AC.", MENU_WHITE_CENTERED);
		return 2;


	//Knight talents
	case TALENT_REPEL:
		addlinetomenu(ent, "Adds chance for projectiles", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to deflect from shield.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MAG_BOOTS:
		addlinetomenu(ent, "Reduces effect of knockback.", MENU_WHITE_CENTERED);
		return 1;
	//case TALENT_LEAP_ATTACK:
		//addlinetomenu(ent, "Adds stun/knockback effect", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "to boost spell when landing.", MENU_WHITE_CENTERED);
		//return 2;
	//case TALENT_MOBILITY:
	//	addlinetomenu(ent, "Reduces boost cooldown", MENU_WHITE_CENTERED);
		//return 1;
	case TALENT_DURABILITY:
		addlinetomenu(ent, "Increases your health", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "per level bonus!", MENU_WHITE_CENTERED);
		return 2;


	//Cleric talents
	case TALENT_BALANCESPIRIT:
		addlinetomenu(ent, "New spirit that can", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "use the skills of both", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "yin and yang spirits.", MENU_WHITE_CENTERED);
		return 3;
	//case TALENT_HOLY_GROUND:
		//addlinetomenu(ent, "Designate an area as", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "holy ground to regenerate", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "teammates (cmd 'holyground').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_UNHOLY_GROUND:
		addlinetomenu(ent, "Designate an area as", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "unholy ground to damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "enemies (cmd 'unholyground').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_BOOMERANG:
		addlinetomenu(ent, "Turns blessed hammers", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "into boomerangs", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'boomerang').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_PURGE:
		addlinetomenu(ent, "Removes curses and grants", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "temporary invincibility", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and immunity to curses", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'purge').", MENU_WHITE_CENTERED);
		return 4;


	//Weaponmaster talents
	case TALENT_BASIC_AMMO_REGEN:
		addlinetomenu(ent, "Basic ammo regeneration.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Regenerates one ammo pack", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "for the weapon in use.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_COMBAT_EXP:
	    addlinetomenu(ent, "Increases physical,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "damage, but reduces", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "resistance.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_TACTICS:
	    addlinetomenu(ent, "Increases your levelup", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "health and armor bonus!", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_SIDEARMS:
	    addlinetomenu(ent, "Gives you additional", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "respawn weapons. Weapon", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "choice is determined by", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "weapon upgrade level.", MENU_WHITE_CENTERED);
		return 4;


	//Necromancer talents
	case TALENT_EVIL_CURSE:
		addlinetomenu(ent, "Increases curse duration.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_CHEAPER_CURSES:
		addlinetomenu(ent, "Reduces curse cost.", MENU_WHITE_CENTERED);
		return 1;
	//case TALENT_CORPULENCE:
		//addlinetomenu(ent, "Increases hellspawn", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "and monster health/armor,", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "but reduces damage. Can't,", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "combine with Life Tap.", MENU_WHITE_CENTERED);
		//return 4;
	case TALENT_LIFE_TAP:
		addlinetomenu(ent, "Increases hellspawn", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and monster damage,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "but slowly saps life.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Can't combine with", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Corpulence.", MENU_WHITE_CENTERED);
		return 5;
	case TALENT_DIM_VISION:
		addlinetomenu(ent, "Adds chance to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "automatically curse", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "enemies that shoot you.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_FLIGHT:
		addlinetomenu(ent, "Reduces jetpack cost.", MENU_WHITE_CENTERED);
		return 1;


	//Shaman talents
	case TALENT_TOTEM:
		addlinetomenu(ent, "Allows you to spawn", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "healthier totems. Totem can not", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "be of the opposite element.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_ICE:
		addlinetomenu(ent, "Allows your water totem", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to damage its targets.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_WIND:
		addlinetomenu(ent, "Allows your air totem to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "ghost attacks for you.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_STONE:
		addlinetomenu(ent, "Allows your earth totem to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "increase your resistance.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_SHADOW:
		addlinetomenu(ent, "Allows your darkness totem", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to let you vamp beyond your", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "maximum health limit.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_PEACE:
		addlinetomenu(ent, "Allows your nature totem to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "regenerate your power cubes.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_VOLCANIC:
		addlinetomenu(ent, "Adds a chance for fire", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "totem to shoot a fireball.", MENU_WHITE_CENTERED);
		return 2;


	//Alien talents

	case TALENT_PHANTOM_OBSTACLE:
		addlinetomenu(ent, "Reduces time to cloak.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_SUPER_HEALER:
		addlinetomenu(ent, "Allows healer to heal", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "beyond maximum health.", MENU_WHITE_CENTERED);
		return 2;

	//case TALENT_FASTMOVE:
		//addlinetomenu(ent, "Allow you to move Faster.", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "when you receive damage.", MENU_WHITE_CENTERED);
		//return 1;
	case TALENT_LIFE_REG:
		addlinetomenu(ent, "Allow you to regenerate.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "	Life 	", MENU_WHITE_CENTERED);
		return 2;

	case TALENT_PHANTOM_COCOON:
		addlinetomenu(ent, "Allows cocoon to cloak.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_SWARMING:
		addlinetomenu(ent, "Increases spore damage.", MENU_WHITE_CENTERED);
		// addlinetomenu(ent, "but reduces damage.", MENU_WHITE_CENTERED); // lol
		return 1;
	//case TALENT_EXPLODING_BODIES:
		//addlinetomenu(ent, "Makes alien-summons'", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "corpses explode.", MENU_WHITE_CENTERED);
		//return 2;


	// Kamikaze talents
	//case TALENT_MARTYR:
		//addlinetomenu(ent, "Creates an explotion", MENU_WHITE_CENTERED);
		//addlinetomenu(ent, "when you die.", MENU_WHITE_CENTERED);
		//return 2;
	case TALENT_BLAST_RESIST:
		addlinetomenu(ent, "Increases defense against", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "radius damage.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MAGMINESELF:
		addlinetomenu(ent, "Gain the ability", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to turn into a living magmine", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "using 'magmine self'.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_INSTANTPROXYS:
		addlinetomenu(ent, "Makes proxys be removed", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "instantly when they explode.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "On level 2, it removes", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "hold time when building them.", MENU_WHITE_CENTERED);
		return 4;
	default:	return 0;
	}
}

void openTalentMenu(edict_t *ent, int talentID)
{
	talent_t *talent	= &ent->myskills.talents.talent[getTalentSlot(ent, talentID)];
	int level			= talent->upgradeLevel;
	int lineCount = 7;//12;

	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, GetTalentString(talentID), MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

	lineCount += writeTalentDescription(ent, talentID);

	addlinetomenu(ent, " ", 0);
	//addlinetomenu(ent, "Current", MENU_GREEN_CENTERED);
	//writeTalentUpgrade(ent, talentID, level);
	addlinetomenu(ent, " ", 0);
	
	if(talent->upgradeLevel < talent->maxLevel)
		addlinetomenu(ent, "Upgrade this talent.", -1*(talentID+1));
	else addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Previous menu.", talentID+1);

	setmenuhandler(ent, TalentUpgradeMenu_handler);
	ent->client->menustorage.currentline = lineCount;
	showmenu(ent);
}

//****************************************
//*********** Main Talent Menu ***********
//****************************************

void openTalentMenu_handler(edict_t *ent, int option)
{
	switch(option)
	{
	case 9999:	//Exit
		{
			closemenu(ent);
			return;
		}
	default: openTalentMenu(ent, option);
	}
}

void OpenTalentUpgradeMenu (edict_t *ent, int lastline)
{
	talent_t	*talent;
	char		buffer[30];
	int			i;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Talents", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = 0; i < ent->myskills.talents.count; i++)
	{
		talent = &ent->myskills.talents.talent[i];

		//create menu string
		strcpy(buffer, GetTalentString(talent->id));
		strcat(buffer, ":");
		padRight(buffer, 15);

		addlinetomenu(ent, va("%d. %s %d/%d", i+1, buffer, talent->upgradeLevel, talent->maxLevel), talent->id);
	}

	// menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("You have %d talent points.", ent->myskills.talents.talentPoints), 0);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Exit", 9999);
	setmenuhandler(ent, openTalentMenu_handler);

	if (!lastline)	ent->client->menustorage.currentline = ent->myskills.talents.count + 6;
	else			ent->client->menustorage.currentline = lastline + 2;

	showmenu(ent);

	// try to shortcut to chat-protect mode
	if (ent->client->idle_frames < CHAT_PROTECT_FRAMES-51)
		ent->client->idle_frames = CHAT_PROTECT_FRAMES-51;
}