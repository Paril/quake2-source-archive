#include "g_local.h"

int checkString(char *input, char test)
{
	int i;
	for (i = 0; input[i] != 0; i++) {
		if (input[i] == test) {
			return 1;
		}
	}
	return 0;
}

int checkName(edict_t *ent, char *name) {
	if (strlen(name) < 3) {
		gi.cprintf(ent, PRINT_HIGH, "Your name must be atleast 3 letters\n");
		return 0;
	}
	if (checkString(name, ' ')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain spaces\n");
		return 0;
	}
	if (checkString(name, '.')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '.'\n");
		return 0;
	}
	if (checkString(name, '/')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '/'\n");
		return 0;
	}
	if (checkString(name, '\\')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '\'\n");
		return 0;
	}
	if (checkString(name, '*')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '*'\n");
		return 0;
	}
	if (checkString(name, '?')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '?'\n");
		return 0;
	}
	if (checkString(name, '$')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '$'\n");
		return 0;
	}
	if (checkString(name, '<')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '<'\n");
		return 0;
	}
	if (checkString(name, '>')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '>'\n");
		return 0;
	}
	if (checkString(name, ':')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain ':'\n");
		return 0;
	}
	if (checkString(name, '|')) {
		gi.cprintf(ent, PRINT_HIGH, "Your name may not contain '|'\n");
		return 0;
	}
	return 1;
}

void createNewSkills(edict_t *ent, int classId) {
	int i;
	if (!ent->client)
		return;
	memset(&ent->client->pers.skills, 0, sizeof(skills_t));

	ent->client->pers.skills.active_powerup = 0;

	ent->client->pers.skills.itemId[0] = 1; //Give Blaster
	if (classId == 0) { //Soldier
		ent->client->pers.skills.itemId[1] = 3; //SSG
		ent->client->pers.skills.itemId[2] = 5; //CG
		ent->client->pers.skills.itemId[3] = 7; //GL
		ent->client->pers.skills.itemId[4] = 8; //RL
		ent->client->pers.skills.itemId[5] = 9; //HB
		ent->client->pers.skills.itemId[6] = 10; //RG
		ent->client->pers.skills.itemId[7] = 11; //Bfg
		ent->client->pers.skills.itemId[8] = 12; //Give him LightArmor

		ent->client->pers.skills.powerup_type[0] = 1; //Weapon damage
		ent->client->pers.skills.powerup_level[0] = 10;
		ent->client->pers.skills.powerup_type[1] = 7; //Projectile speed
		ent->client->pers.skills.powerup_level[1] = 3;
	} else if (classId == 1) { //Tank
		ent->client->pers.skills.itemId[1] = 2; //SG
		ent->client->pers.skills.itemId[2] = 3; //SSG
		ent->client->pers.skills.itemId[3] = 4; //MG
		ent->client->pers.skills.itemId[4] = 8; //RL
		ent->client->pers.skills.itemId[5] = 10; //RG
		ent->client->pers.skills.itemId[6] = 12; //Light armor
		ent->client->pers.skills.itemId[7] = 34; //Give him CombatArmor

		ent->client->pers.skills.powerup_type[0] = 27; //Max armor
		ent->client->pers.skills.powerup_level[0] = 10;
		ent->client->pers.skills.powerup_type[1] = 23; //Armor eff
		ent->client->pers.skills.powerup_level[1] = 3;
		ent->client->pers.skills.powerup_type[2] = 31; //Armor regen
		ent->client->pers.skills.powerup_level[2] = 2;
	} else if (classId == 2) { //Cleric
		ent->client->pers.skills.itemId[1] = 2; //SG
		ent->client->pers.skills.itemId[2] = 4; //MG
		ent->client->pers.skills.itemId[3] = 6; //HG
		ent->client->pers.skills.itemId[4] = 19; //Shotty
		ent->client->pers.skills.itemId[5] = 12; //Give him LightArmor
		ent->client->pers.skills.itemId[6] = 27; //Give him LifeVest

		ent->client->pers.skills.powerup_type[0] = 26; //Max health
		ent->client->pers.skills.powerup_level[0] = 10;
		ent->client->pers.skills.powerup_type[1] = 56; //Create health
		ent->client->pers.skills.powerup_level[1] = 5;
		ent->client->pers.skills.powerup_type[2] = 57; //Create armor
		ent->client->pers.skills.powerup_level[2] = 5;
		ent->client->pers.skills.powerup_type[3] = 66; //Regen aura
		ent->client->pers.skills.powerup_level[3] = 5;
	} else if (classId == 3) { //Mage
		ent->client->pers.skills.itemId[1] = 2; //SG
		ent->client->pers.skills.itemId[2] = 4; //MG
		ent->client->pers.skills.itemId[3] = 9; //HB
		ent->client->pers.skills.itemId[4] = 22; //Wand
		ent->client->pers.skills.itemId[5] = 12; //Give him LightArmor

		ent->client->pers.skills.powerup_type[0] = 47; //Fireball
		ent->client->pers.skills.powerup_level[0] = 5;
		ent->client->pers.skills.powerup_type[1] = 53; //Plague bomb
		ent->client->pers.skills.powerup_level[1] = 5;
		ent->client->pers.skills.powerup_type[2] = 50; //Lightning
		ent->client->pers.skills.powerup_level[2] = 5;
	} else if (classId == 4) { //Vampire
		ent->client->pers.skills.itemId[1] = 2; //SG
		ent->client->pers.skills.itemId[2] = 3; //SSG
		ent->client->pers.skills.itemId[3] = 4; //MG
		ent->client->pers.skills.itemId[4] = 9; //HB
		ent->client->pers.skills.itemId[5] = 140; //Arm

		ent->client->pers.skills.powerup_type[0] = 1; //Weapon damage
		ent->client->pers.skills.powerup_level[0] = 10;
	}

	ent->client->pers.skills.wornItem[0] = -1;
	ent->client->pers.skills.activatingItem[0] = 0; //Make him wield the Blaster
	for (i = 1; i < GIEX_BASEITEMS; i++) {
		ent->client->pers.skills.wornItem[i] = -1;
		ent->client->pers.skills.activatingItem[i] = -1;
	}
	ent->client->pers.itemchanging = -1; // set client ready to equip items
	updateItemLevels(ent);
//	info = getItemInfo(1);
//	ent->client->newweapon = info->item;

	ApplyMax(ent);
	ent->health = ent->max_health;
	ent->client->magic = ent->client->max_magic;
}

int newCharacter(edict_t *ent, int classId, char *password) {
	FILE *file;
	char path[128];
	char tmppwd[32];

	strncpy(tmppwd, password, 16);
	createNewSkills(ent, classId);
	sprintf(path, "%s/%s.ccs", charpath->string, ent->client->pers.netname);
	if ((file = fopen(path, "rb")) == NULL) {
		if (strlen(password) < 3) {
			gi.cprintf(ent, PRINT_HIGH, "You must supply a password of at least 3 letters.\n");
			ent->client->pers.loggedin = false;
			ApplyMax(ent);
			return 0;
		}
		ent->client->pers.skills.activeClass = classId;
		ent->radius_dmg = 0;
		ent->count = 0;

		ent->client->pers.expRemain = getClassExpLeft(ent, ent->client->pers.skills.activeClass);
		ent->client->pers.itemchanging = -1; // set client ready to equip items

		strncpy(ent->client->pers.skills.password, tmppwd, 16);
		gi.cprintf(ent, PRINT_HIGH, "Creating new user with password %s\n", ent->client->pers.skills.password);
		ent->client->pers.loggedin = true;
		ApplyMax(ent);
		strncpy(ent->client->pers.skills.password + 16, ent->client->pers.netname, 16);
		saveCharacter(ent);
		return 1;
	} else {
		gi.cprintf(ent, PRINT_HIGH, "Character already exists. Please choose another name.\n");
		ent->client->pers.loggedin = false;
		ApplyMax(ent);
		return 0;
	}
}

int loadCharacter(edict_t *ent, char *password) {
	FILE *file = NULL;
	char path[128];
	char tmppwd[32];
	int test=120, i/*, i2, pu*/;
/*	powerupinfo_t *info;
	iteminfo_t *iteminfo;*/


	if (!ent->client)
		return 0;

	if (!checkName(ent, ent->client->pers.netname)) {
		return 0;
	}
	strncpy(tmppwd, password, 16);
	createNewSkills(ent, 0);
	sprintf(path, "%s/%s.ccs", charpath->string, ent->client->pers.netname);
	file = fopen(path, "rb");
	if (file == NULL) {
		gi.cprintf(ent, PRINT_HIGH, "No such character, use \"Create character\"\n");
		createNewSkills(ent, 0);
		ent->client->pers.loggedin = false;
		ApplyMax(ent);
		return 0;
	}
	fread(&test, sizeof(coop->value), 1, file);
	if (test != coop->value) {
		if (!test)
			gi.cprintf(ent, PRINT_HIGH, "This character may not be used in cooperative mode\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "This character may only be used in cooperative mode\n");
		createNewSkills(ent, 0);
		ent->client->pers.loggedin = false;
		fclose(file);
		ApplyMax(ent);
		return 0;
	}
	fread(&ent->client->pers.skills, sizeof(skills_t), 1, file);
	if (strncmp(ent->client->pers.skills.password, password, 16)) {
		char msg[128];
		sprintf(msg, "%s failed login attempt.\n", ent->client->pers.netname);
		logmsg(msg);
		gi.cprintf(ent, PRINT_HIGH, "Wrong password\n");
		createNewSkills(ent, 0);
		ent->client->pers.loggedin = false;
		fclose(file);
		ApplyMax(ent);
		return 0;
	}
	fread(ent->client->pers.inventory, sizeof(ent->client->pers.inventory), 1, file);
	ent->client->pers.loggedin = true;
	fclose(file);
	ApplyMax(ent);
	ent->health = ent->max_health;
	ent->client->magic = ent->client->max_magic;

// Check for bad powerups
// in Stash

/*
	for (i = 0; i < GIEX_PUCARRYPERCHAR; i++) {
		powerupinfo_t *puinfo;
		if (ent->client->pers.skills.powerup_type[i] == 0) {
			break;
		}
		puinfo = getPowerupInfo(ent->client->pers.skills.powerup_type[i]);
		if (!puinfo->inuse) {
			break;
		}
		if (ent->client->pers.skills.powerup_level[i] > puinfo->maxlevel) {
			gi.dprintf("lowering %s (%d in stash)\n", puinfo->name, i);
			ent->client->pers.skills.powerup_level[i] = puinfo->maxlevel;
		}
	}

// in Memory
	for (i = 0; i < GIEX_PUPERCHAR; i++) {
		powerupinfo_t *puinfo;
		if (ent->client->pers.skills.putype[i] == 0) {
			break;
		}
		puinfo = getPowerupInfo(ent->client->pers.skills.putype[i]);
		if (!puinfo->inuse) {
		   break;
		}
		if (puinfo->isspell < 1) {
			ent->client->pers.skills.putype[i] = 0;
			ent->client->pers.skills.pucurlvl[i] = 0;
			ent->client->pers.skills.pumaxlvl[i] = 0;
			ent->client->pers.skills.puexp[i] = 0;
			gi.dprintf("killing %s (%d in memory)\n", puinfo->name, i);
			continue;
		}*
		if (ent->client->pers.skills.pucurlvl[i] > puinfo->maxlevel) {
			ent->client->pers.skills.pucurlvl[i] = puinfo->maxlevel;
			gi.dprintf("lowering curlvl for %s (%d in memory)\n", puinfo->name, i);
		}
		if (ent->client->pers.skills.pumaxlvl[i] > puinfo->maxlevel) {
			ent->client->pers.skills.pumaxlvl[i] = puinfo->maxlevel;
			gi.dprintf("lowering maxlvl for %s (%d in memory)\n", puinfo->name, i);
		}
	}

// in Items
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		iteminfo_t *iteminfo;
		if (ent->client->pers.skills.itemId[i] == 0) {
			break;
		}
		iteminfo = getItemInfo(ent->client->pers.skills.itemId[i]);
		if (iteminfo->name[0] == '\0') {
			break;
		}
		for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
			powerupinfo_t *puinfo;
			int puinfoslot;
			if (ent->client->pers.skills.item_putype[i][pu] == 0) {
				break;
			}
			puinfo = getPowerupInfo(ent->client->pers.skills.item_putype[i][pu]);
			if (!puinfo->inuse) {
				break;
			}
			puinfoslot = getItemPowerupInfoSlot(iteminfo, ent->client->pers.skills.item_putype[i][pu]);
			if (puinfoslot < 0) { // Incompatible powerup socketed
				ent->client->pers.skills.item_putype[i][pu] = 0;
				ent->client->pers.skills.item_pucurlvl[i][pu] = 0;
				ent->client->pers.skills.item_pumaxlvl[i][pu] = 0;
				ent->client->pers.skills.item_puexp[i][pu] = 0;
				gi.dprintf("killing %s in %s\n", puinfo->name, iteminfo->name);
			}
			if (ent->client->pers.skills.item_pucurlvl[i][pu] > puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot]) {
				ent->client->pers.skills.item_pucurlvl[i][pu] = puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot];
				gi.dprintf("lowering curlvl for %s in %s\n", puinfo->name, iteminfo->name);
			}
			if (ent->client->pers.skills.item_pumaxlvl[i][pu] > puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot]) {
				ent->client->pers.skills.item_pumaxlvl[i][pu] = puinfo->maxlevel + iteminfo->pumaxlvl[puinfoslot];
				gi.dprintf("lowering maxlvl for %s in %s\n", puinfo->name, iteminfo->name);
			}
		}
	}
*/

//Check sanity of worn weapon
	if (ent->client->pers.skills.wornItem[0] == -1) {
		for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
			iteminfo_t *iteminfo = getItemInfo(ent->client->pers.skills.itemId[i]);
			if ((iteminfo->type & 255) == GIEX_ITEM_WEAPON) {
				ent->client->pers.skills.wornItem[0] = i;
				break;
			}
		}
	}
//Only re-equip weapon.
/*	for (i = 0; i < 1; i++) {
		ent->client->pers.skills.activatingItem[i] = ent->client->pers.skills.wornItem[i]; //Make him re-wield everything
		ent->client->pers.skills.wornItem[i] = -1;
		updateItemLevels(ent);
	}*/

	updateItemLevels(ent);
//Recalculate all exp requirements
	ent->client->pers.expRemain = getClassExpLeft(ent, ent->client->pers.skills.activeClass);
	for (i = 0; i < GIEX_PUPERCHAR; i++) {
		if (ent->client->pers.skills.putype[i] == 0) {
			break;
		}
		ent->client->pers.puexpRemain[i] = getPowerupCost(ent, i);
	}
	for (i = 0; i < GIEX_ITEMPERCHAR; i++) {
		int pu;
		if (ent->client->pers.skills.itemId[i] == 0) {
			break;
		}
		for (pu = 0; pu < GIEX_PUPERITEM; pu++) {
			if (ent->client->pers.skills.item_putype[i][pu] == 0) {
				break;
			}
			ent->client->pers.item_puexpRemain[i][pu] = getItemPowerupCost(ent, i, pu);
		}
	}

	for (i = 0; i < GIEX_BASEITEMS; i++) {
		ent->client->pers.skills.activatingItem[i] = -1;
	}
	ent->client->pers.itemchanging = -1; // set client ready to equip items

	strncpy(ent->client->pers.skills.password + 16, ent->client->pers.netname, 16);
	return 1;
}

void saveCharacter(edict_t *ent)
{
	FILE *file;
	char path[128];

	if (!ent->client)
		return;
	if (!ent->client->pers.loggedin) {
		gi.cprintf(ent, PRINT_HIGH, "You are not logged in!\n");
		return;
	}
	if (!checkName(ent, ent->client->pers.skills.password + 16)) {
		return;
	}

	gi.cprintf(ent, PRINT_HIGH, "Saving character \"%s\".\n", ent->client->pers.skills.password + 16);
	sprintf(path, "%s/%s.ccs", charpath->string, ent->client->pers.skills.password + 16);
	if ((file = fopen(path, "wb")) == NULL)
		return;
	if (coop->value) {
		int tmp=1;
		fwrite(&tmp, sizeof(int), 1, file);
	} else {
		int tmp=0;
		fwrite(&tmp, sizeof(int), 1, file);
	}
	fwrite(&ent->client->pers.skills, sizeof(skills_t), 1, file);
	fwrite(ent->client->pers.inventory, sizeof(ent->client->pers.inventory), 1, file);
	fclose(file);
}
