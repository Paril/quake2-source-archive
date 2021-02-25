#include "g_local.h"
#include "g_main.h"

/******* KnightMare's cvar code file ***********/
cvar_t	*allow_inven;		//whether to show inventory alongside SOG menu

cvar_t	*old_guys_allow_armor_stacking;  //whether idg1, idg2, and idge classes can stack armor


cvar_t	*old_guys_same_armor_bonus;  //whether the old guys get the same armor bonus piclup value as the idg4 players
cvar_t	*old_guys_same_health_bonus;  //whether the old guys get the same health bonus piclup value as the idg4 players

cvar_t	*idg1_pack_increase; //whether Wolfie gets his ammo capacites increased to Q2 levels when picking up a pack
cvar_t	*idg3_pack_increase; //whether Axe gets his ammo capacites increased to Q2 levels when picking up a pack

cvar_t 	*idg3_inv_armor_nodamage; //set to 1 for Axe to take no damage to armor when he has pent
cvar_t 	*idg3_allow_health_bonus; //set to 1 to allow Axe pickup stimpacks/potions over 100 health

cvar_t	*replace_megasphere; //whether to replace the idg2 megasphere (which acts like a Doom soulsphere, adding 100 health when it should add 200 health and 200 armor) with the soulsphere that acts like a megasphere

//idg1 maximums
cvar_t	*idg1_max_health;
cvar_t	*idg1_soul_health;
cvar_t	*idg1_max_armor;
cvar_t	*idg1_max_bullets;
cvar_t	*idg1_max_shells;
cvar_t	*idg1_max_rockets;
cvar_t	*idg1_max_grenades;
cvar_t	*idg1_max_cells;
cvar_t	*idg1_max_slugs;

//idg2 maximums
cvar_t	*idg2_max_health;
cvar_t	*idg2_soul_health;
cvar_t	*idg2_max_armor;
cvar_t	*idg2_max_bullets;
cvar_t	*idg2_max_shells;
cvar_t	*idg2_max_rockets;
cvar_t	*idg2_max_grenades;
cvar_t	*idg2_max_cells;
cvar_t	*idg2_max_slugs;

//idg3 maximums
cvar_t	*idg3_max_health;
cvar_t	*idg3_soul_health;
cvar_t	*idg3_max_armor;
cvar_t	*idg3_max_bullets;
cvar_t	*idg3_max_shells;
cvar_t	*idg3_max_rockets;
cvar_t	*idg3_max_grenades;
cvar_t	*idg3_max_cells;
cvar_t	*idg3_max_slugs;

//idg4 maximums
cvar_t	*idg4_max_health;
cvar_t	*idg4_soul_health;
cvar_t	*idg4_max_armor;
cvar_t	*idg4_max_bullets;
cvar_t	*idg4_max_shells;
cvar_t	*idg4_max_rockets;
cvar_t	*idg4_max_grenades;
cvar_t	*idg4_max_cells;
cvar_t	*idg4_max_slugs;

// maximum settings if a player gets a pack
cvar_t	*pack_bullets;
cvar_t	*pack_shells;
cvar_t	*pack_rockets;
cvar_t	*pack_grenades;
cvar_t	*pack_cells;
cvar_t	*pack_slugs;

//idg2 backpack settings
cvar_t	*idg2_pack_bullets;
cvar_t	*idg2_pack_shells;
cvar_t	*idg2_pack_rockets;
cvar_t	*idg2_pack_cells;

// maximum settings if a player gets a bandolier
cvar_t	*bando_bullets;
cvar_t	*bando_shells;
cvar_t	*bando_cells;
cvar_t	*bando_slugs;
cvar_t  *bando_rockets;

cvar_t	*health_bonus_value; //value of stimpacks
cvar_t	*armor_bonus_value; //value of armor shards

cvar_t	*idg1_chalice_armor_bonus_value; //value of chalice
cvar_t	*idg1_chest_armor_bonus_value; //value of chest
cvar_t	*idg1_crown_armor_bonus_value; //value of crown

cvar_t	*powerup_max;

//duration of powerups
cvar_t	*quad_time;
cvar_t	*inv_time;
cvar_t	*breather_time;
cvar_t	*enviro_time;
cvar_t	*invis_time;
cvar_t	*silencer_shots;

void lithium_defaults(void)
{
	allow_inven = gi.cvar("allow_inven", "0", 0);

	old_guys_allow_armor_stacking = gi.cvar("old_guys_allow_armor_stacking", "0", 0);

	old_guys_same_armor_bonus = gi.cvar("old_guys_same_armor_bonus", "0", 0);
	old_guys_same_health_bonus = gi.cvar("old_guys_same_health_bonus", "0", 0);

	idg1_pack_increase = gi.cvar("idg1_pack_increase", "0", 0);
	idg3_pack_increase = gi.cvar("idg3_pack_increase", "0", 0);

	idg3_inv_armor_nodamage = gi.cvar("idg3_inv_armor_nodamage", "0", 0);
	idg3_allow_health_bonus = gi.cvar("idg3_allow_health_bonus", "0", 0);
	replace_megasphere = gi.cvar("replace_megasphere", "0", 0);

	//idg1 maximums
	idg1_max_health = gi.cvar("idg1_max_health", "100", 0);
	idg1_soul_health = gi.cvar("idg1_soul_health", "200", 0);
	idg1_max_armor = gi.cvar("idg1_max_armor", "200", 0);
	idg1_max_bullets = gi.cvar("idg1_max_bullets", "300", 0);
	idg1_max_shells = gi.cvar("idg1_max_shells", "100", 0);
	idg1_max_rockets = gi.cvar("idg1_max_rockets", "100", 0);
	idg1_max_grenades = gi.cvar("idg1_max_grenades", "0", 0);
	idg1_max_cells = gi.cvar("idg1_max_cells", "200", 0);
	idg1_max_slugs = gi.cvar("idg1_max_slugs", "0", 0);

	//idg2 maximums
	idg2_max_health = gi.cvar("idg2_max_health", "100", 0);
	idg2_soul_health = gi.cvar("idg2_soul_health", "200", 0);
	idg2_max_armor = gi.cvar("idg2_max_armor", "300", 0);
	idg2_max_bullets = gi.cvar("idg2_max_bullets", "200", 0);
	idg2_max_shells = gi.cvar("idg2_max_shells", "50", 0);
	idg2_max_rockets = gi.cvar("idg2_max_rockets", "50", 0);
	idg2_max_grenades = gi.cvar("idg2_max_grenades", "0", 0);
	idg2_max_cells = gi.cvar("idg2_max_cells", "300", 0);
	idg2_max_slugs = gi.cvar("idg2_max_slugs", "0", 0);

	//idg3 maximums
	idg3_max_health = gi.cvar("idg3_max_health", "100", 0);
	idg3_soul_health = gi.cvar("idg3_soul_health", "200", 0);
	idg3_max_armor = gi.cvar("idg3_max_armor", "200", 0);
	idg3_max_bullets = gi.cvar("idg3_max_bullets", "200", 0);
	idg3_max_shells = gi.cvar("idg3_max_shells", "100", 0);
	idg3_max_rockets = gi.cvar("idg3_max_rockets", "100", 0);
	idg3_max_grenades = gi.cvar("idg3_max_grenades", "0", 0);
	idg3_max_cells = gi.cvar("idg3_max_cells", "100", 0);
	idg3_max_slugs = gi.cvar("idg3_max_slugs", "0", 0);

	//idg4 maximums
	idg4_max_health = gi.cvar("idg4_max_health", "100", 0);
	idg4_soul_health = gi.cvar("idg4_soul_health", "200", 0);
	idg4_max_armor = gi.cvar("idg4_max_armor", "200", 0);
	idg4_max_bullets = gi.cvar("idg4_max_bullets", "200", 0);
	idg4_max_shells = gi.cvar("idg4_max_shells", "100", 0);
	idg4_max_rockets = gi.cvar("idg4_max_rockets", "50", 0);
	idg4_max_grenades = gi.cvar("idg4_max_grenades", "50", 0);
	idg4_max_cells = gi.cvar("idg4_max_cells", "200", 0);
	idg4_max_slugs = gi.cvar("idg4_max_slugs", "50", 0);

	// maximum settings if a player gets a pack
	pack_bullets = gi.cvar("pack_bullets", "300", 0);
	pack_shells = gi.cvar("pack_shells", "200", 0);
	pack_rockets = gi.cvar("pack_rockets", "100", 0);
	pack_grenades = gi.cvar("pack_grenades", "100", 0);
	pack_cells = gi.cvar("pack_cells", "300", 0);
	pack_slugs = gi.cvar("pack_slugs", "100", 0);

	//idg2 backpack settings
	idg2_pack_bullets = gi.cvar("idg2_pack_bullets", "400", 0);
	idg2_pack_shells = gi.cvar("idg2_pack_shells", "100", 0);
	idg2_pack_rockets = gi.cvar("idg2_pack_rockets", "100", 0);
	idg2_pack_cells = gi.cvar("idg2_pack_cells", "600", 0);

	// maximum settings if a player gets a bandolier
	bando_bullets = gi.cvar("bando_bullets", "250", 0);
	bando_shells = gi.cvar("bando_shells", "150", 0);
	bando_cells = gi.cvar("bando_cells", "250", 0);
	bando_slugs = gi.cvar("bando_slugs", "75", 0);
	bando_rockets = gi.cvar("bando_rockets", "50", 0);

	health_bonus_value= gi.cvar("health_bonus_value", "2", 0);
	armor_bonus_value = gi.cvar("armor_bonus_value", "2", 0);

	idg1_chalice_armor_bonus_value  = gi.cvar("idg1_chalice_armor_bonus_value", "2", 0);
	idg1_chest_armor_bonus_value  = gi.cvar("idg1_chest_armor_bonus_value", "5", 0);
	idg1_crown_armor_bonus_value  = gi.cvar("idg1_crown_armor_bonus_value", "10", 0);

	powerup_max = gi.cvar("powerup_max", "2", 0);

	//duration of powerups
	quad_time = gi.cvar("quad_time", "30", 0);
	inv_time = gi.cvar("inv_time", "30", 0);
	breather_time = gi.cvar("breather_time", "30", 0);
	enviro_time = gi.cvar("enviro_time", "30", 0);
	invis_time = gi.cvar("invis_time", "30", 0);
	silencer_shots = gi.cvar("silencer_shots", "30", 0);


}