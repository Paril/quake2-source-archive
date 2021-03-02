/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include "g_local.h"

lvar_t *first_lvar = NULL, *last_lvar = NULL;

lvar_t *lvar(char *name, char *string, char *edit, int cat) {
	lvar_t *lvar = (lvar_t *)malloc(sizeof(lvar_t));
	cvar_t *cvar = gi.cvar(name, string, 0);

	lvar->cvar = cvar;
	lvar->string = cvar->string;
	lvar->value = cvar->value;
	lvar->admin = 50;
	lvar->edit = edit;
	lvar->cat = cat;

	if(first_lvar) {
		last_lvar->next = lvar;
		last_lvar = lvar;
	}
	else {
		first_lvar = lvar;
		last_lvar = lvar;
	}

	lvar->func = NULL;
	lvar->next = NULL;

	return lvar;
}

extern lvar_t *use_runes, *use_hook, *use_nocamp, *use_mapvote, *use_highscores, *use_zbotkick;

void Var_SetFeatures(void) {
	char features[256] = "";
	
	if(use_runes->value) strlcat(features, "Runes, ", sizeof(features));
	if(use_hook->value) strlcat(features, "Hook, ", sizeof(features));
	if(use_nocamp->value) strlcat(features, "NoCamp, ", sizeof(features));
	if(use_mapvote->value) strlcat(features, "MapVote, ", sizeof(features));
	if(use_highscores->value) strlcat(features, "HiScores, ", sizeof(features));
	if(use_zbotkick->value) strlcat(features, "ZbotKick, ", sizeof(features));

	if(strlen(features))
		features[strlen(features) - 2] = 0;

	gi.cvar("features", features, CVAR_SERVERINFO);
	gi.cvar_set("features", features);
}

void Var_RunFrame(void) {
	lvar_t *lvar = first_lvar;
	while(lvar) {
		if(lvar->cvar->modified) {
//			if(!strchr(lvar->cvar->name, '[')
//				gi.bprintf(PRINT_MEDIUM, "Admin changed \"%s\" to \"%s\".\n", lvar->cvar->name, lvar->cvar->string);
			lvar->value = lvar->cvar->value;
			lvar->string = lvar->cvar->string;
			if(lvar->func)
				lvar->func();
			Menu_LVarChanged(lvar);
			lvar->cvar->modified = false;

			Var_SetFeatures();
		}
		lvar = lvar->next;
	}
}

void Var_Shutdown(void) {
	lvar_t *del, *lvar = first_lvar;
	while(lvar) {
		del = lvar;
		lvar = lvar->next;
		free(del);
	}
	first_lvar = NULL;
	last_lvar = NULL;
}

void pvar(edict_t *ent, char *name, int *ptr, char *edit) {
	pvar_t *pvar = (pvar_t *)gi.TagMalloc(sizeof(pvar_t), TAG_LEVEL);

	pvar->name = name;
	pvar->value = ptr;
	pvar->old = *ptr;
	pvar->edit = edit;

	if(ent->pvar) {
		pvar_t *next = ent->pvar;
		while(next) {
			if(next->next) {
				next = next->next;
				continue;
			}
			next->next = pvar;
			break;
		}
	}
	else
		ent->pvar = pvar;
}

pvar_t *pvar_find(edict_t *ent, char *name) {
	pvar_t *pvar = ent->pvar;
	while(pvar) {
		if(!strcmp(name, pvar->name))
			return pvar;
		pvar = pvar->next;
	}
	return pvar;
}

lvar_t *jacket_armor;
lvar_t *combat_armor;
lvar_t *body_armor;
lvar_t *shard_armor;
lvar_t *small_health;

lvar_t *power_armor_screen;
lvar_t *power_armor_shield;

lvar_t *start_health;
lvar_t *start_armor;
lvar_t *start_bullets;
lvar_t *start_shells;
lvar_t *start_rockets;
lvar_t *start_grenades;
lvar_t *start_cells;
lvar_t *start_slugs;

lvar_t *max_health;
lvar_t *max_armor;
lvar_t *max_bullets;
lvar_t *max_shells;
lvar_t *max_rockets;
lvar_t *max_grenades;
lvar_t *max_cells;
lvar_t *max_slugs;

lvar_t *start_weapon;
lvar_t *start_blaster;
lvar_t *start_shotgun;
lvar_t *start_sshotgun;
lvar_t *start_machinegun;
lvar_t *start_chaingun;
lvar_t *start_grenadelauncher;
lvar_t *start_rocketlauncher;
lvar_t *start_hyperblaster;
lvar_t *start_railgun;
lvar_t *start_bfg;

lvar_t *blaster_damage;
lvar_t *blaster_speed;
lvar_t *shotgun_damage;
lvar_t *shotgun_count;
lvar_t *shotgun_hspread;
lvar_t *shotgun_vspread;
lvar_t *sshotgun_damage;
lvar_t *sshotgun_count;
lvar_t *sshotgun_hspread;
lvar_t *sshotgun_vspread;
lvar_t *machinegun_damage;
lvar_t *machinegun_hspread;
lvar_t *machinegun_vspread;
lvar_t *chaingun_damage;
lvar_t *chaingun_hspread;
lvar_t *chaingun_vspread;
lvar_t *grenade_damage;
lvar_t *grenade_radius;
lvar_t *rocket_damage;
lvar_t *rocket_damage2;
lvar_t *rocket_rdamage;
lvar_t *rocket_radius;
lvar_t *rocket_speed;
lvar_t *rocket_knockback;
lvar_t *hyperblaster_damage;
lvar_t *hyperblaster_speed;
lvar_t *railgun_damage;
lvar_t *bfg_damage;
lvar_t *bfg_radius;
lvar_t *bfg_speed;
lvar_t *bfg_raydamage;
lvar_t *bfg_balldamage;
lvar_t *bfg_ballradius;

lvar_t *no_shotgun;
lvar_t *no_sshotgun;
lvar_t *no_machinegun;
lvar_t *no_chaingun;
lvar_t *no_grenadelauncher;
lvar_t *no_rocketlauncher;
lvar_t *no_hyperblaster;
lvar_t *no_railgun;
lvar_t *no_bfg;

lvar_t *no_pent;
lvar_t *no_quad;

lvar_t *knockback_adjust;
lvar_t *knockback_self;
lvar_t *fall_damagemod;

lvar_t *ping_watch;
lvar_t *ping_endwatch;
lvar_t *ping_min;
lvar_t *ping_max;

lvar_t *chatmuzzle_says;
lvar_t *chatmuzzle_time;
lvar_t *chatmuzzle_holdtime;
lvar_t *chatkick_says;
lvar_t *chatkick_time;
lvar_t *chatkick_muzzles;

lvar_t *dmflag[16];

extern gitem_armor_t jacketarmor_info;
extern gitem_armor_t combatarmor_info;
extern gitem_armor_t bodyarmor_info;

void jackarmor_changed(void) {
	Armor_Realize(&jacketarmor_info, jacket_armor->string);
}

void combatarmor_changed(void) {
	Armor_Realize(&combatarmor_info, combat_armor->string);
}

void bodyarmor_changed(void) {
	Armor_Realize(&bodyarmor_info, body_armor->string);
}

void Var_InitLevel(void) {
}

void dmflags_change(void) {
	int i;
	for(i = 0; i < 16; i++) {
		char name[16];
		snprintf(name, sizeof(name), "dmflag[%d]", i);
		if((int)dmflags->value & 1 << i)
			gi.cvar_set(name, "1");
		else
			gi.cvar_set(name, "0");
	}
}

void dmflag_change(void) {
	int flags = (int)dmflags->value;
	char fl[8];
	int i;

	for(i = 0; i < 16; i++)
		if(dmflag[i]->value)
			flags |= 1 << i;
		else
			flags &= ~(1 << i);

	snprintf(fl, sizeof(fl), "%d", flags);
	gi.cvar_set(dmflags->name, fl);
}

void Var_InitGame(void) {
	int i;
	lvar_t *l;

	for(i = 0; i < 16; i++) {
		char name[16];
		snprintf(name, sizeof(name), "dmflag[%d]", i);
		dmflag[i] = lvar(name, "0", "^", VAR_NONE);
		dmflag[i]->func = dmflag_change;
	}
	l = lvar("dmflags", "0", 0, VAR_NONE);
	l->func = dmflags_change;
	dmflags_change();

	lvar("public", "1", "^", VAR_GENERAL);
	lvar("timelimit", "30", "###", VAR_GENERAL);
	lvar("fraglimit", "50", "###", VAR_GENERAL);
	lvar("sv_maxvelocity", "2000", "####", VAR_GENERAL);
	lvar("sv_gravity", "800", "###", VAR_GENERAL);
	lvar("sv_airaccelerate", "10", "##", VAR_GENERAL);

	// armor vars
	jacket_armor = lvar("jacket_armor", "50 100 .30 .00", 0, VAR_OTHER);
	combat_armor = lvar("combat_armor", "75 150 .60 .30", 0, VAR_OTHER);
	body_armor = lvar("body_armor", "100 200 .80 .60", 0, VAR_OTHER);
	shard_armor = lvar("shard_armor", "5", "#", VAR_OTHER);
	small_health = lvar("small_health", "2", "#", VAR_OTHER);

	jacket_armor->func = jackarmor_changed;
	combat_armor->func = combatarmor_changed;
	body_armor->func = bodyarmor_changed;

	// power armor vars
	power_armor_screen = lvar("power_armor_screen", "0.25", "#.##", VAR_OTHER);
	power_armor_shield = lvar("power_armor_shield", "0.50", "#.##", VAR_OTHER);

	// ammo vars
	start_health = lvar("start_health", "100", "###", VAR_START);
	start_armor = lvar("start_armor", "0", "###", VAR_START);
	start_bullets = lvar("start_bullets", "50", "###", VAR_START);
	start_shells = lvar("start_shells", "10", "###", VAR_START);
	start_rockets = lvar("start_rockets", "5", "###", VAR_START);
	start_grenades = lvar("start_grenades", "1", "###", VAR_START);
	start_cells = lvar("start_cells", "50", "###", VAR_START);
	start_slugs = lvar("start_slugs", "5", "###", VAR_START);

	max_health = lvar("max_health", "120", "###", VAR_MAX);
	max_armor = lvar("max_armor", "200", "###", VAR_MAX);
	max_bullets = lvar("max_bullets", "240", "###", VAR_MAX);
	max_shells = lvar("max_shells", "120", "###", VAR_MAX);
	max_rockets = lvar("max_rockets", "60", "###", VAR_MAX);
	max_grenades = lvar("max_grenades", "60", "###", VAR_MAX);
	max_cells = lvar("max_cells", "240", "###", VAR_MAX);
	max_slugs = lvar("max_slugs", "60", "###", VAR_MAX);

	// weapon vars
	start_weapon = lvar("start_weapon", "1", "Auto-Pick:Blaster:Shotgun:S. Shotgun:Machinegun:Chaingun:G. Launcher:R. Launcher:H. Blaster:Railgun:BFG 10k", VAR_STARTWEAP);
	start_blaster = lvar("start_blaster", "1", "^", VAR_STARTWEAP);
	start_shotgun = lvar("start_shotgun", "0", "^", VAR_STARTWEAP);
	start_sshotgun = lvar("start_sshotgun", "0", "^", VAR_STARTWEAP);
	start_machinegun = lvar("start_machinegun", "0", "^", VAR_STARTWEAP);
	start_chaingun = lvar("start_chaingun", "0", "^", VAR_STARTWEAP);
	start_grenadelauncher = lvar("start_grenadelauncher", "0", "^", VAR_STARTWEAP);
	start_rocketlauncher = lvar("start_rocketlauncher", "0", "^", VAR_STARTWEAP);
	start_hyperblaster = lvar("start_hyperblaster", "0", "^", VAR_STARTWEAP);
	start_railgun = lvar("start_railgun", "0", "^", VAR_STARTWEAP);
	start_bfg = lvar("start_bfg", "0", "^", VAR_STARTWEAP);

	blaster_damage = lvar("blaster_damage", "17", "##", VAR_WEAPON);
	blaster_speed = lvar("blaster_speed", "1000", "####", VAR_WEAPON);

	shotgun_damage = lvar("shotgun_damage", "6", "##", VAR_WEAPON);
	shotgun_count = lvar("shotgun_count", "11", "##", VAR_WEAPON);
	shotgun_hspread = lvar("shotgun_hspread", "500", "####", VAR_WEAPON);
	shotgun_vspread = lvar("shotgun_vspread", "500", "####", VAR_WEAPON);
	
	sshotgun_damage = lvar("sshotgun_damage", "6", "##", VAR_WEAPON);
	sshotgun_count = lvar("sshotgun_count", "22", "##", VAR_WEAPON);
	sshotgun_hspread = lvar("sshotgun_hspread", "1000", "####", VAR_WEAPON);
	sshotgun_vspread = lvar("sshotgun_vspread", "500", "####", VAR_WEAPON);
	
	machinegun_damage = lvar("machinegun_damage", "8", "##", VAR_WEAPON);
	machinegun_hspread = lvar("machinegun_hspread", "300", "####", VAR_WEAPON);
	machinegun_vspread = lvar("machinegun_vspread", "500", "####", VAR_WEAPON);
	
	chaingun_damage = lvar("chaingun_damage", "8", "##", VAR_WEAPON);
	chaingun_hspread = lvar("chaingun_hspread", "300", "####", VAR_WEAPON);
	chaingun_vspread = lvar("chaingun_vspread", "500", "####", VAR_WEAPON);
	
	grenade_damage = lvar("grenade_damage", "100", "###", VAR_WEAPON);
	grenade_radius = lvar("grenade_radius", "140", "###", VAR_WEAPON);
	
	rocket_damage = lvar("rocket_damage", "100", "###", VAR_WEAPON);
	rocket_damage2 = lvar("rocket_damage2", "20", "##", VAR_WEAPON);
	rocket_rdamage = lvar("rocket_rdamage", "120", "###", VAR_WEAPON);
	rocket_radius = lvar("rocket_radius", "140", "###", VAR_WEAPON);
	rocket_speed = lvar("rocket_speed", "900", "####", VAR_WEAPON);
	rocket_knockback = lvar("rocket_knockback", "0", "^", VAR_WEAPON);
		
	hyperblaster_damage = lvar("hyperblaster_damage", "17", "##", VAR_WEAPON);
	hyperblaster_speed = lvar("hyperblaster_speed", "1000", "####", VAR_WEAPON);
	
	railgun_damage = lvar("railgun_damage", "120", "###", VAR_WEAPON);
	
	bfg_damage = lvar("bfg_damage", "200", "###", VAR_WEAPON);
	bfg_radius = lvar("bfg_radius", "1000", "####", VAR_WEAPON);
	bfg_speed = lvar("bfg_speed", "400", "####", VAR_WEAPON);
	bfg_raydamage = lvar("bfg_raydamage", "5", "##", VAR_WEAPON);
	bfg_balldamage = lvar("bfg_balldamage", "200", "##", VAR_WEAPON);
	bfg_ballradius = lvar("bfg_ballradius", "100", "##", VAR_WEAPON);

	no_shotgun = lvar("no_shotgun", "0", "^", VAR_INHIBIT);
	no_sshotgun = lvar("no_sshotgun", "0", "^", VAR_INHIBIT);
	no_machinegun = lvar("no_machinegun", "0", "^", VAR_INHIBIT);
	no_chaingun = lvar("no_chaingun", "0", "^", VAR_INHIBIT);
	no_grenadelauncher = lvar("no_grenadelauncher", "0", "^", VAR_INHIBIT);
	no_rocketlauncher = lvar("no_rocketlauncher", "0", "^", VAR_INHIBIT);
	no_hyperblaster = lvar("no_hyperblaster", "0", "^", VAR_INHIBIT);
	no_railgun = lvar("no_railgun", "0", "^", VAR_INHIBIT);
	no_bfg = lvar("no_bfg", "1", "^", VAR_INHIBIT);

	no_quad = lvar("no_quad", "0", "^", VAR_INHIBIT);
	no_pent = lvar("no_pent", "1", "^", VAR_INHIBIT);

	knockback_adjust = lvar("knockback_adjust", "1.0", "#.##", VAR_OTHER);
	knockback_self = lvar("knockback_self", "3.2", "#.##", VAR_OTHER);
	fall_damagemod = lvar("fall_damagemod", "1.0", "#.##", VAR_OTHER);

	ping_watch = lvar("ping_watch", "10", "###", VAR_OTHER);
	ping_endwatch = lvar("ping_endwatch", "20", "###", VAR_OTHER);
	ping_min = lvar("ping_min", "0", "###", VAR_OTHER);
	ping_max = lvar("ping_max", "1000", "###", VAR_OTHER);

	chatmuzzle_says = lvar("chatmuzzle_says", "3", "#", VAR_OTHER);
	chatmuzzle_time = lvar("chatmuzzle_time", "2.0", "#.#", VAR_OTHER);
	chatmuzzle_holdtime = lvar("chatmuzzle_holdtime", "20", "##", VAR_OTHER);
	chatkick_says = lvar("chatkick_says", "10", "#", VAR_OTHER);
	chatkick_time = lvar("chatkick_time", "5.0", "#.#", VAR_OTHER);
	chatkick_muzzles = lvar("chatkick_muzzles", "3", "##", VAR_OTHER);

	Var_SetFeatures();
}

void Var_PutClientInServer(edict_t *ent) {
	ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] = start_bullets->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] = start_shells->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] = start_rockets->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] = start_grenades->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] = start_cells->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] = start_slugs->value;

	ent->client->pers.inventory[ITEM_INDEX(FindItem("blaster"))] = start_blaster->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))] = start_shotgun->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] = start_sshotgun->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] = start_machinegun->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] = start_chaingun->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("grenade launcher"))] = start_grenadelauncher->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] = start_rocketlauncher->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] = start_hyperblaster->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] = start_railgun->value;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg10k"))] = start_bfg->value;

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Jacket Armor"))] = start_armor->value;

	if(!ctf_grapple->value || !ctf->value)
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 0;
}

void Var_InitClientPersistant(gclient_t *client) {
	client->pers.health = start_health->value;
	client->pers.max_health = max_health->value;
	client->pers.max_bullets = max_bullets->value;
	client->pers.max_shells = max_shells->value;
	client->pers.max_rockets = max_rockets->value;
	client->pers.max_grenades = max_grenades->value;
	client->pers.max_cells = max_cells->value;
	client->pers.max_slugs = max_slugs->value;
}

void Var_ClientThink(edict_t *ent) {
	int time, pingavg;

	if(level.intermissiontime)
		return;
	if(!ent->inuse)
		return;
	if(ent->client->disconnecting)
		return;

	time = level.framenum - ent->client->resp.enterframe;
	if(!time)
		return;

	if(time < ping_watch->value * 10 || time > ping_endwatch->value * 10)
		return;

	pingavg = ent->ping_total / ent->ping_count;
	if(pingavg < ping_min->value || (ping_max->value && pingavg > ping_max->value)) {
		gi.cprintf(ent, PRINT_HIGH, "\nServer requires your average ping be between %d and %d.  "
			"Yours is %d.  Sorry, disconnecting you.\n\n", (int)ping_min->value, (int)ping_max->value, pingavg);
		gi.AddCommandString(va("kick %d\n", ent - g_edicts - 1));
		ent->client->disconnecting = true;
	}
}

qboolean Var_SpawnEntity(edict_t *ent) {
	if(!strcmp(ent->classname, "item_pack")) return false;

	if(no_shotgun->value && !strcmp(ent->classname, "weapon_shotgun")) return false;
	if(no_sshotgun->value && !strcmp(ent->classname, "weapon_supershotgun")) return false;
	if(no_shotgun->value && no_sshotgun->value && !strcmp(ent->classname, "ammo_shells")) return false;

	if(no_machinegun->value && !strcmp(ent->classname, "weapon_machinegun")) return false;
	if(no_chaingun->value && !strcmp(ent->classname, "weapon_chaingun")) return false;
	if(no_machinegun->value && no_chaingun->value && !strcmp(ent->classname, "ammo_bullets")) return false;

	if(no_grenadelauncher->value && !strcmp(ent->classname, "weapon_grenadelauncher")) return false;
	if(no_grenadelauncher->value && !strcmp(ent->classname, "ammo_grenades")) return false;

	if(no_rocketlauncher->value && !strcmp(ent->classname, "weapon_rocketlauncher")) return false;
	if(no_rocketlauncher->value && !strcmp(ent->classname, "ammo_rockets")) return false;

	if(no_hyperblaster->value && !strcmp(ent->classname, "weapon_hyperblaster")) return false;
	if(no_bfg->value && !strcmp(ent->classname, "weapon_bfg")) return false;
	if(no_hyperblaster->value && no_bfg->value && !strcmp(ent->classname, "ammo_cells")) return false;

	if(no_railgun->value && !strcmp(ent->classname, "weapon_railgun")) return false;

	if(no_quad->value && !strcmp(ent->classname, "item_quad")) return false;
	if(no_pent->value && !strcmp(ent->classname, "item_invulnerability")) return false;

	return true;
}
