#include "g_local.h"

//Knightmare added
#include "g_sogcvar.h"


typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;


void SP_item_health (edict_t *self);
void SP_item_health_small (edict_t *self);
void SP_item_health_large (edict_t *self);
void SP_item_health_mega (edict_t *self);

void SP_info_player_start (edict_t *ent);
void SP_info_player_deathmatch (edict_t *ent);
void SP_info_player_coop (edict_t *ent);
void SP_info_player_intermission (edict_t *ent);

void SP_func_plat (edict_t *ent);
void SP_func_rotating (edict_t *ent);
void SP_func_button (edict_t *ent);
void SP_func_door (edict_t *ent);
void SP_func_door_secret (edict_t *ent);
void SP_func_door_rotating (edict_t *ent);
void SP_func_water (edict_t *ent);
void SP_func_train (edict_t *ent);
void SP_func_conveyor (edict_t *self);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void SP_func_explosive (edict_t *self);
void SP_func_timer (edict_t *self);
void SP_func_areaportal (edict_t *ent);
void SP_func_clock (edict_t *ent);
void SP_func_killbox (edict_t *ent);

void SP_trigger_always (edict_t *ent);
void SP_trigger_once (edict_t *ent);
void SP_trigger_multiple (edict_t *ent);
void SP_trigger_relay (edict_t *ent);
void SP_trigger_push (edict_t *ent);
void SP_trigger_hurt (edict_t *ent);
void SP_trigger_key (edict_t *ent);
void SP_trigger_counter (edict_t *ent);
void SP_trigger_elevator (edict_t *ent);
void SP_trigger_gravity (edict_t *ent);
void SP_trigger_monsterjump (edict_t *ent);

void SP_target_temp_entity (edict_t *ent);
void SP_target_speaker (edict_t *ent);
void SP_target_explosion (edict_t *ent);
void SP_target_changelevel (edict_t *ent);
void SP_target_secret (edict_t *ent);
void SP_target_goal (edict_t *ent);
void SP_target_splash (edict_t *ent);
void SP_target_spawner (edict_t *ent);
void SP_target_blaster (edict_t *ent);
void SP_target_crosslevel_trigger (edict_t *ent);
void SP_target_crosslevel_target (edict_t *ent);
void SP_target_laser (edict_t *self);
void SP_target_help (edict_t *ent);
void SP_target_actor (edict_t *ent);
void SP_target_lightramp (edict_t *self);
void SP_target_earthquake (edict_t *ent);
void SP_target_character (edict_t *ent);
void SP_target_string (edict_t *ent);

void SP_worldspawn (edict_t *ent);
void SP_viewthing (edict_t *ent);

void SP_light (edict_t *self);
void SP_light_mine1 (edict_t *ent);
void SP_light_mine2 (edict_t *ent);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_path_corner (edict_t *self);
void SP_point_combat (edict_t *self);

void SP_misc_explobox (edict_t *self);
void SP_misc_banner (edict_t *self);
void SP_misc_satellite_dish (edict_t *self);
void SP_misc_actor (edict_t *self);
void SP_misc_gib_arm (edict_t *self);
void SP_misc_gib_leg (edict_t *self);
void SP_misc_gib_head (edict_t *self);
void SP_misc_insane (edict_t *self);
void SP_misc_deadsoldier (edict_t *self);
void SP_misc_viper (edict_t *self);
void SP_misc_viper_bomb (edict_t *self);
void SP_misc_bigviper (edict_t *self);
void SP_misc_strogg_ship (edict_t *self);
void SP_misc_teleporter (edict_t *self);
void SP_misc_teleporter_dest (edict_t *self);
void SP_misc_blackhole (edict_t *self);
void SP_misc_eastertank (edict_t *self);
void SP_misc_easterchick (edict_t *self);
void SP_misc_easterchick2 (edict_t *self);

void SP_monster_berserk (edict_t *self);
void SP_monster_gladiator (edict_t *self);
void SP_monster_gunner (edict_t *self);
void SP_monster_infantry (edict_t *self);
void SP_monster_soldier_light (edict_t *self);
void SP_monster_soldier (edict_t *self);
void SP_monster_soldier_ss (edict_t *self);
void SP_monster_tank (edict_t *self);
void SP_monster_medic (edict_t *self);
void SP_monster_flipper (edict_t *self);
void SP_monster_chick (edict_t *self);
void SP_monster_parasite (edict_t *self);
void SP_monster_flyer (edict_t *self);
void SP_monster_brain (edict_t *self);
void SP_monster_floater (edict_t *self);
void SP_monster_hover (edict_t *self);
void SP_monster_mutant (edict_t *self);
void SP_monster_supertank (edict_t *self);
void SP_monster_boss2 (edict_t *self);
void SP_monster_jorg (edict_t *self);
void SP_monster_boss3_stand (edict_t *self);

void SP_monster_commander_body (edict_t *self);

void SP_turret_breach (edict_t *self);
void SP_turret_base (edict_t *self);
void SP_turret_driver (edict_t *self);



//=========================================

// Skid - added 

//=========================================

//Quake Health

//=========================================

void SP_item_q1_health (edict_t *self);

void SP_item_q1_health_large (edict_t *self);

void SP_item_q1_health_mega (edict_t *self);

// Doom Health

//=========================================

void SP_item_d_health (edict_t *self);

void SP_item_d_health_large (edict_t *self);

void SP_item_d_health_small (edict_t *self);

void SP_item_d_health_mega (edict_t *self);

// Arno added - WOLF Health

//=========================================

void SP_item_idg1_healthS (edict_t *self);

void SP_item_idg1_healthM (edict_t *self);

void SP_item_idg1_healthL (edict_t *self);

void SP_item_idg1_megahealth (edict_t *self);

// Misc Quake1 Entities

//=========================================

void SP_q1_misc_large_flame(edict_t *self);

void SP_q1_misc_small_flame (edict_t *self);

void SP_q1_misc_torch(edict_t *self); 

void SP_idg3_target_lavaball(edict_t *self);

void SP_q1_misc_globe(edict_t *self);

void SP_idg3_target_trap(edict_t *self);

void SP_misc_idg0_keen(edict_t *self);

void SP_misc_idg2_bigtorch(edict_t *self);

void SP_misc_idg2_bigtree(edict_t *self);

void SP_misc_idg2_candleabra(edict_t *self);

void SP_misc_idg2_actor(edict_t *self);

void SP_misc_idg1_armored(edict_t *self);

void SP_misc_idg1_cageskel(edict_t *self);

void SP_misc_idg1_flag(edict_t *self);

void SP_misc_idg1_barrel(edict_t *self);

void SP_misc_idg1_barrelwide(edict_t *self);

void SP_misc_idg1_bed(edict_t *self);

void SP_misc_idg1_plant(edict_t *self);

void SP_misc_idg1_skelhang(edict_t *self);

void SP_misc_idg1_cage(edict_t *self);

void SP_misc_idg1_skeleton(edict_t *self);

void SP_misc_idg1_pot(edict_t *self);



//=========================================

// QUAKE1 MONSTERS BAY-BEEEEEE

//=========================================


//=========================================

// DOOM MONSTERS

//=========================================

void SP_monster_idg2_super (edict_t *self);

void SP_monster_idg2_cu (edict_t *self);

void SP_monster_idg2_monkey (edict_t *self);

void SP_monster_idg2_pinky (edict_t *self);

void SP_monster_idg2_dude (edict_t *self);

void SP_monster_idg2_sarge (edict_t *self);

void SP_monster_idg2_head (edict_t *self);

void SP_monster_idg2_skull (edict_t *self);

void SP_monster_idg2_medic (edict_t *self);

void SP_monster_idg2_skeleton (edict_t *self);

void SP_monster_idg2_commando (edict_t *self);

void SP_monster_idg2_fatdude (edict_t *self);

void SP_monster_idg2_spider (edict_t *self);

void SP_monster_idg2_skullspawner (edict_t *self);

void SP_monster_idg2_spiderchild (edict_t *self);

//=========================================

// WOLF MONSTERS   

//=========================================

void SP_monster_idg1_sa (edict_t *self);

void SP_monster_idg1_ss (edict_t *self);

void SP_monster_idg1_dog (edict_t *self);

void SP_monster_idg1_bd19 (edict_t *self);

void SP_monster_idg1_officer (edict_t *self);

void SP_monster_idg1_mutant (edict_t *self);

void SP_monster_idg1_doctor (edict_t *self);

void SP_monster_idg1_commander (edict_t *self);

void SP_monster_idg1_general (edict_t *self);

void SP_monster_idg1_priest (edict_t *self);

//=========================================

void SP_d_misc_barrel(edict_t *self);



spawn_t	spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},



//Skid added

//Arno - replaced generations-entity-names with SOG-entity-names

//=========================================

	{"item_idg3_healthS", SP_item_q1_health},

	{"item_idg3_healthL", SP_item_q1_health_large},

	{"item_idg3_megahealth", SP_item_q1_health_mega},



	{"item_idg2_healthM", SP_item_d_health},

	{"item_idg2_healthS", SP_item_d_health_small},

	{"item_idg2_healthL", SP_item_d_health_large},

	{"item_idg2_megahealth", SP_item_d_health_mega},

// Arno added - WOLF Health Items

	{"item_idg1_healthS", SP_item_idg1_healthS},

	{"item_idg1_healthM", SP_item_idg1_healthM},

	{"item_idg1_healthL", SP_item_idg1_healthL},    

	{"item_idg1_megahealth", SP_item_idg1_megahealth}, 

//=========================================

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},

//ZOID

	{"info_player_team1", SP_info_player_team1},

	{"info_player_team2", SP_info_player_team2},

//ZOID

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_door_secret", SP_func_door_secret},
	{"func_door_rotating", SP_func_door_rotating},
	{"func_rotating", SP_func_rotating},
	{"func_train", SP_func_train},
	{"func_water", SP_func_water},
	{"func_conveyor", SP_func_conveyor},
	{"func_areaportal", SP_func_areaportal},
	{"func_clock", SP_func_clock},
	{"func_wall", SP_func_wall},
	{"func_object", SP_func_object},
	{"func_timer", SP_func_timer},
	{"func_explosive", SP_func_explosive},
	{"func_killbox", SP_func_killbox},

	{"trigger_always", SP_trigger_always},
	{"trigger_once", SP_trigger_once},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_relay", SP_trigger_relay},
	{"trigger_push", SP_trigger_push},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_key", SP_trigger_key},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_monsterjump", SP_trigger_monsterjump},
	
	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_changelevel", SP_target_changelevel},
	{"target_secret", SP_target_secret},
	{"target_goal", SP_target_goal},
	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_laser", SP_target_laser},
	{"target_help", SP_target_help},
	{"target_actor", SP_target_actor},
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

// Skid added

	{"target_idg3_trap", SP_idg3_target_trap},
	{"target_idg3_lavaball", SP_idg3_target_lavaball},

// end Skid

	{"worldspawn", SP_worldspawn},
	{"viewthing", SP_viewthing},

	{"light", SP_light},
	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_null", SP_info_null},
	{"func_group", SP_info_null},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},
	{"point_combat", SP_point_combat},

	{"misc_explobox", SP_misc_explobox},
	{"misc_banner", SP_misc_banner},
	{"misc_satellite_dish", SP_misc_satellite_dish},
	{"misc_actor", SP_misc_actor},
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
	{"misc_insane", SP_misc_insane},
	{"misc_deadsoldier", SP_misc_deadsoldier},
	{"misc_viper", SP_misc_viper},
	{"misc_viper_bomb", SP_misc_viper_bomb},
	{"misc_bigviper", SP_misc_bigviper},
	{"misc_strogg_ship", SP_misc_strogg_ship},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_blackhole", SP_misc_blackhole},
	{"misc_eastertank", SP_misc_eastertank},
	{"misc_easterchick", SP_misc_easterchick},
	{"misc_easterchick2", SP_misc_easterchick2},

//ZOID

	{"misc_ctf_banner", SP_misc_ctf_banner},

	{"misc_ctf_small_banner", SP_misc_ctf_small_banner},

//ZOID



// Skid added

//=========================================

	{"misc_idg0_keen", SP_misc_idg0_keen},
	
	{"misc_idg3_large_flame", SP_q1_misc_large_flame},

	{"misc_idg3_small_flame", SP_q1_misc_small_flame},

	{"misc_idg3_torch", SP_q1_misc_torch},

	{"misc_idg3_light_globe", SP_q1_misc_globe},

	{"misc_idg2_barrel",SP_d_misc_barrel},
	
	{"misc_idg2_bigtorch",SP_misc_idg2_bigtorch},
	
	{"misc_idg2_bigtree",SP_misc_idg2_bigtree},

	{"misc_idg2_candleabra",SP_misc_idg2_candleabra},
	
	{"misc_idg2_actor",SP_misc_idg2_actor},
	
	{"misc_idg1_armored",SP_misc_idg1_armored},
	
	{"misc_idg1_cageskel",SP_misc_idg1_cageskel},
	
	{"misc_idg1_flag",SP_misc_idg1_flag},
	
	{"misc_idg1_barrel",SP_misc_idg1_barrel},
	
	{"misc_idg1_barrelwide",SP_misc_idg1_barrelwide},
	
	{"misc_idg1_bed",SP_misc_idg1_bed},
	
	{"misc_idg1_plant",SP_misc_idg1_plant},
	
	{"misc_idg1_skelhang",SP_misc_idg1_skelhang},
	
	{"misc_idg1_cage",SP_misc_idg1_cage},
	
	{"misc_idg1_skeleton",SP_misc_idg1_skeleton},
	
	{"misc_idg1_pot",SP_misc_idg1_pot},

//=========================================

	{"monster_berserk", SP_monster_berserk},
	{"monster_gladiator", SP_monster_gladiator},
	{"monster_gunner", SP_monster_gunner},
	{"monster_infantry", SP_monster_infantry},
	{"monster_soldier_light", SP_monster_soldier_light},
	{"monster_soldier", SP_monster_soldier},
	{"monster_soldier_ss", SP_monster_soldier_ss},
	{"monster_tank", SP_monster_tank},
	{"monster_tank_commander", SP_monster_tank},
	{"monster_medic", SP_monster_medic},
	{"monster_flipper", SP_monster_flipper},
	{"monster_chick", SP_monster_chick},
	{"monster_parasite", SP_monster_parasite},
	{"monster_flyer", SP_monster_flyer},
	{"monster_brain", SP_monster_brain},
	{"monster_floater", SP_monster_floater},
	{"monster_hover", SP_monster_hover},
	{"monster_mutant", SP_monster_mutant},
	{"monster_supertank", SP_monster_supertank},
	{"monster_boss2", SP_monster_boss2},
	{"monster_boss3_stand", SP_monster_boss3_stand},
	{"monster_jorg", SP_monster_jorg},

	{"monster_idg2_super",SP_monster_idg2_super},

	{"monster_idg2_uberdog",SP_monster_idg2_cu},

	{"monster_idg2_pinky",SP_monster_idg2_pinky},

	{"monster_idg2_monkey",SP_monster_idg2_monkey},

	{"monster_idg2_dude", SP_monster_idg2_dude},
	
	{"monster_idg2_sarge", SP_monster_idg2_sarge},
	
	{"monster_idg2_head", SP_monster_idg2_head},
	
	{"monster_idg2_skull", SP_monster_idg2_skull},
	
	{"monster_idg2_medic", SP_monster_idg2_medic},
	
	{"monster_idg2_skeleton", SP_monster_idg2_skeleton},
	
	{"monster_idg2_commando", SP_monster_idg2_commando},
	
	{"monster_idg2_fatdude", SP_monster_idg2_fatdude},
	
	{"monster_idg2_spider", SP_monster_idg2_spider},
	
	{"monster_idg2_skullspawner", SP_monster_idg2_skullspawner},
	
	{"monster_idg2_spiderchild", SP_monster_idg2_spiderchild},


//=========================================

	{"monster_idg1_sa",SP_monster_idg1_sa},

    {"monster_idg1_ss",SP_monster_idg1_ss},

    {"monster_idg1_dog",SP_monster_idg1_dog},

    {"monster_idg1_bd19",SP_monster_idg1_bd19},

    {"monster_idg1_officer",SP_monster_idg1_officer},
    
    {"monster_idg1_mutant",SP_monster_idg1_mutant},
    
    {"monster_idg1_doctor",SP_monster_idg1_doctor},
    
    {"monster_idg1_commander",SP_monster_idg1_commander},
    
    {"monster_idg1_general",SP_monster_idg1_general},
    
    {"monster_idg1_priest",SP_monster_idg1_priest},


//=========================================


	{"monster_commander_body", SP_monster_commander_body},

	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_driver", SP_turret_driver},



	{NULL, NULL}
};

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn (edict_t *ent)
{
	spawn_t	*s;
	gitem_t	*item;
	int		i;

	if (!ent->classname)
	{
		gi.dprintf ("ED_CallSpawn: NULL classname\n");
		return;
	}
	//Knightmare- replace the wacked idg2 megasphere that acts like a soulphere 
	//with the old megasphere if stacking is enabled
	if (replace_megasphere->value)
		if (!strcmp(ent->classname, "item_idg2_megahealth"))
			ent->classname = "item_idg2_soulsphere";

	// check item spawn functions
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		if (!strcmp(item->classname, ent->classname))
		{	// found it
			SpawnItem (ent, item);
			return;
		}
	}

	// check normal spawn functions
	for (s=spawns ; s->name ; s++)
	{
		if (!strcmp(s->name, ent->classname))
		{	// found it
			s->spawn (ent);
			return;
		}
	}
	gi.dprintf ("%s doesn't have a spawn function\n", ent->classname);
}

/*
=============
ED_NewString
=============
*/
char *ED_NewString (char *string)
{
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = gi.TagMalloc (l, TAG_LEVEL);

	new_p = newb;

	for (i=0 ; i< l ; i++)
	{
		if (string[i] == '\\' && i < l-1)
		{
			i++;
			if (string[i] == 'n')
				*new_p++ = '\n';
			else
				*new_p++ = '\\';
		}
		else
			*new_p++ = string[i];
	}
	
	return newb;
}




/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
void ED_ParseField (char *key, char *value, edict_t *ent)
{
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for (f=fields ; f->name ; f++)
	{
		if (!(f->flags & FFL_NOSPAWN) && !Q_stricmp(f->name, key))
		{	// found it
			if (f->flags & FFL_SPAWNTEMP)
				b = (byte *)&st;
			else
				b = (byte *)ent;

			switch (f->type)
			{
			case F_LSTRING:
				*(char **)(b+f->ofs) = ED_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	gi.dprintf ("%s is not a field\n", key);
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
char *ED_ParseEdict (char *data, edict_t *ent)
{
	qboolean	init;
	char		keyname[256];
	char		*com_token;

	init = false;
	memset (&st, 0, sizeof(st));

// go through all the dictionary pairs
	while (1)
	{	
	// parse key
		com_token = COM_Parse (&data);
		if (com_token[0] == '}')
			break;
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		strncpy (keyname, com_token, sizeof(keyname)-1);
		
	// parse value	
		com_token = COM_Parse (&data);
		if (!data)
			gi.error ("ED_ParseEntity: EOF without closing brace");

		if (com_token[0] == '}')
			gi.error ("ED_ParseEntity: closing brace without data");

		init = true;	

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (keyname[0] == '_')
			continue;

		ED_ParseField (keyname, com_token, ent);
	}

	if (!init)
		memset (ent, 0, sizeof(*ent));

	return data;
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams (void)
{
	edict_t	*e, *e2, *chain;
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for (i=1, e=g_edicts+i ; i < globals.num_edicts ; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->team)
			continue;
		if (e->flags & FL_TEAMSLAVE)
			continue;
		chain = e;
		e->teammaster = e;
		c++;
		c2++;
		for (j=i+1, e2=e+1 ; j < globals.num_edicts ; j++,e2++)
		{
			if (!e2->inuse)
				continue;
			if (!e2->team)
				continue;
			if (e2->flags & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team, e2->team))
			{
				c2++;
				chain->teamchain = e2;
				e2->teammaster = e;
				chain = e2;
				e2->flags |= FL_TEAMSLAVE;
			}
		}
	}

	gi.dprintf ("%i teams with %i entities\n", c, c2);
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/



//Skid Added

//qboolean CheckMapEntFile(char *mapname);


void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;

	skill_level = floor (skill->value);
	if (skill_level < 0)
		skill_level = 0;
	if (skill_level > 3)
		skill_level = 3;
	if (skill->value != skill_level)
		gi.cvar_forceset("skill", va("%f", skill_level));

	SaveClientData ();

	gi.FreeTags (TAG_LEVEL);

	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);



	// set client fields on player ents
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;


// Skid added

	gi.cvar_forceset("gen_ctf","0");

// parse ents
	while (1)
	{
		// parse the opening brace	
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

		if (!ent)
			ent = g_edicts;
		else
			ent = G_Spawn ();
		entities = ED_ParseEdict (entities, ent);

		// yet another map hack
		if (!Q_stricmp(level.mapname, "command") && !Q_stricmp(ent->classname, "trigger_once") && !Q_stricmp(ent->model, "*27"))
			ent->spawnflags &= ~SPAWNFLAG_NOT_HARD;

		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH )
				{
					G_FreeEdict (ent);	
					inhibit++;
					continue;
				}
			}
			else
			{
				if ( /* ((coop->value) && (ent->spawnflags & SPAWNFLAG_NOT_COOP)) || */
					((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->value == 2) || (skill->value == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict (ent);	
						inhibit++;
						continue;
					}
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}	

	gi.dprintf ("%i entities inhibited\n", inhibit);

#ifdef DEBUG
	i = 1;
	ent = EDICT_NUM(i);
	while (i < globals.num_edicts) {
		if (ent->inuse != 0 || ent->inuse != 1)
			Com_DPrintf("Invalid entity %d\n", i);
		i++, ent++;
	}
#endif

	G_FindTeams ();

	PlayerTrail_Init ();





/////////////////////////////////////

// CTF STUFF- Skid added

/////////////////////////////////////

	if(sog_team->value || sog_ctf->value)

	{

		gi.imageindex("ctfidg1");

		gi.imageindex("ctfidg2");

		gi.imageindex("ctfidg3");

		gi.imageindex("ctfidg4");

	}

	

	if (sog_ctf->value) 

	{

		//precaches

		gi.imageindex("ctfsb1");

		gi.imageindex("ctfsb2");

		gi.imageindex("i_ctf1");

		gi.imageindex("i_ctf2");

		gi.imageindex("i_ctf1d");

		gi.imageindex("i_ctf2d");

		gi.imageindex("i_ctf1t");

		gi.imageindex("i_ctf2t");

		gi.imageindex("i_ctfj");


		gi.configstring (CS_STATUSBAR, ctf_statusbar);

		CTFSetupTechSpawn();

	} 

//End Skid

}


//===================================================================

#if 0
	// cursor positioning
	xl <value>
	xr <value>
	yb <value>
	yt <value>
	xv <value>
	yv <value>

	// drawing
	statpic <name>
	pic <stat>
	num <fieldwidth> <stat>
	string <stat>

	// control
	if <stat>
	ifeq <stat> <value>
	ifbit <stat> <value>
	endif

#endif

char *single_statusbar = 

"yb	-24 "



// health

"xv	0 "

"hnum "

"xv	50 "

"yb	-24 " //

"pic 0 "

"yb	-24 "

//"pic 0 "



// ammo

"if 2 "

"	xv	100 "

"	anum "

"	xv	150 "

"	pic 2 "

"endif "



// armor

"if 4 "

"	xv	200 "

"	rnum "

"	xv	250 "

"	pic 4 "

"endif "



// selected item

"yb	-24 "



"if 6 "

"	xv	288 " //296 - Doom item pics - Skid

"	pic 6 "

"endif "



"yb	-50 "



// picked up item

"if 7 "

"	xv	0 "

"	pic 7 "

"	xv	26 "

"	yb	-42 "

"	stat_string 8 "

"	yb	-50 "

"endif "



// timer

"if 9 "

"	xv	262 "

"	num	2	10 "

"	xv	296 "

"	pic	9 "

"endif "



//  help / weapon icon 

"if 11 "

"	xv	148 "

"	pic	11 "

"endif "

;



//FIXME

//CHASECAM etc



char *dm_statusbar =

"yb	-24 "



// health

"xv	0 "

"hnum "

"xv	50 "

"yb	-24 "

"pic 0 "

"yb	-24 "



// ammo

"if 2 "

"	xv	100 "

"	anum "

"	xv	150 "

"	pic 2 "

"endif "



// armor

"if 4 "

"	xv	200 "

"   yb	-24 "

"	rnum "

"	xv	250 "

"	pic 4 "

"endif "





"yb	-24 "



// selected item

"if 6 "

"	xv	288 " //296 - Doom item pics - Skid

"	pic 6 "

"endif "



"yb	-50 "



// picked up item

"if 7 "

"	xv	0 "

"	pic 7 "

"	xv	26 "

"	yb	-42 "

"	stat_string 8 "

"	yb	-50 "

"endif "



// timer

"if 9 "

"	xv	246 "

"	num	2	10 "

"	xv	296 "

"	pic	9 "

"endif "



//  help / weapon icon 

"if 11 "

"	xv	148 "

"	pic	11 "

"endif "



//  frags

"xr	-50 "

"yt 2 "

"num 3 14"



// id view state

/*"if 27 "

  "xv 0 "

  "yb -58 "

  "string \"Viewing\" "

  "xv 64 "

  "stat_string 27 "

"endif "

*/

;





/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
void SP_worldspawn (edict_t *ent)
{
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1

	//---------------

	// reserve some spots for dead player bodies for coop / deathmatch
	InitBodyQue ();

	// set configstrings for items
	SetItemNames ();

	if (st.nextmap)
		strcpy (level.nextmap, st.nextmap);

	// make some data visible to the server

	if (ent->message && ent->message[0])
	{
		gi.configstring (CS_NAME, ent->message);
		strncpy (level.level_name, ent->message, sizeof(level.level_name));
	}
	else
		strncpy (level.level_name, level.mapname, sizeof(level.level_name));



//Skid added

//Get Extra info from WorldSpawn

	if(ent->style)

		level.game = ent->style;

	else

		level.game = CLASS_Q2;


	if(((int)sogflags->value & SOG_IDG3_ONLY) && 

		(level.game != CLASS_Q1))

	{

		if(random() > 0.4)

			gi.configstring (CS_SKY, "dold_");	

		else

			gi.configstring (CS_SKY, "swamp2");

	}

	else if(((int)sogflags->value & SOG_IDG2_ONLY) && 

			(level.game != CLASS_DOOM))

	{

		if(random() > 0.4)

			gi.configstring (CS_SKY, "lava");	

		else

			gi.configstring (CS_SKY, "hellish");

	}

	else if(((int)sogflags->value & SOG_IDG1_ONLY) && 

			(level.game != CLASS_DOOM))

	{

		if(random() > 0.4)

			gi.configstring (CS_SKY, "gaswks");	

		else

			gi.configstring (CS_SKY, "snow");

	}

	else

	{

// end Skid

		if (st.sky && st.sky[0])
			gi.configstring (CS_SKY, st.sky);
		else
			gi.configstring (CS_SKY, "unit1_");

		gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );
		gi.configstring (CS_SKYAXIS, va("%f %f %f",
					st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	}



	gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );
	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

	// status bar program
	if (deathmatch->value)

	{

		//Skid added

		if(sog_ctf->value)

			gi.configstring (CS_STATUSBAR, ctf_statusbar);

		else

			gi.configstring (CS_STATUSBAR, dm_statusbar);

	}

	else

		gi.configstring (CS_STATUSBAR, single_statusbar);

	//---------------


	// help icon for statusbar
	gi.imageindex ("i_help");
	level.pic_health = gi.imageindex ("i_health");
	gi.imageindex ("help");
	gi.imageindex ("field_3");

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "800");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	snd_fry = gi.soundindex ("player/fry.wav");	// standing in lava / slime

	PrecacheItem (FindItem ("Blaster"));


	gi.soundindex ("player/lava1.wav");
	gi.soundindex ("player/lava2.wav");
	gi.soundindex ("misc/pc_up.wav");
	gi.soundindex ("misc/talk1.wav");
	gi.soundindex ("misc/udeath.wav");

	// gibs
	gi.soundindex ("items/respawn1.wav");

	// sexed sounds
	gi.soundindex ("*death1.wav");
	gi.soundindex ("*death2.wav");
	gi.soundindex ("*death3.wav");
	gi.soundindex ("*death4.wav");
	gi.soundindex ("*fall1.wav");
	gi.soundindex ("*fall2.wav");	
	gi.soundindex ("*gurp1.wav");		// drowning damage
	gi.soundindex ("*gurp2.wav");	
	gi.soundindex ("*jump1.wav");		// player jump
	gi.soundindex ("*pain25_1.wav");
	gi.soundindex ("*pain25_2.wav");
	gi.soundindex ("*pain50_1.wav");
	gi.soundindex ("*pain50_2.wav");
	gi.soundindex ("*pain75_1.wav");
	gi.soundindex ("*pain75_2.wav");
	gi.soundindex ("*pain100_1.wav");
	gi.soundindex ("*pain100_2.wav");

	
	// sexed models
	// THIS ORDER MUST MATCH THE DEFINES IN g_local.h
	// you can add more, max 15
	gi.modelindex ("#w_blaster.md2");
	gi.modelindex ("#w_shotgun.md2");
	gi.modelindex ("#w_sshotgun.md2");
	gi.modelindex ("#w_machinegun.md2");
	gi.modelindex ("#w_chaingun.md2");
	gi.modelindex ("#a_grenades.md2");
	gi.modelindex ("#w_glauncher.md2");
	gi.modelindex ("#w_rlauncher.md2");
	gi.modelindex ("#w_hyperblaster.md2");
	gi.modelindex ("#w_railgun.md2");
	gi.modelindex ("#w_bfg.md2");

	gi.modelindex ("#w_grapple.md2");

	//-------------------

	gi.soundindex ("player/gasp1.wav");		// gasping for air
	gi.soundindex ("player/gasp2.wav");		// head breaking surface, not gasping

	gi.soundindex ("player/watr_in.wav");	// feet hitting water
	gi.soundindex ("player/watr_out.wav");	// feet leaving water

	gi.soundindex ("player/watr_un.wav");	// head going underwater
	
	gi.soundindex ("player/u_breath1.wav");
	gi.soundindex ("player/u_breath2.wav");

	gi.soundindex ("items/pkup.wav");		// bonus item pickup
	gi.soundindex ("world/land.wav");		// landing thud
	gi.soundindex ("misc/h2ohit1.wav");		// landing splash

	gi.soundindex ("items/damage.wav");
	gi.soundindex ("items/protect.wav");
	gi.soundindex ("items/protect4.wav");
	gi.soundindex ("weapons/noammo.wav");

	gi.soundindex ("infantry/inflies1.wav");



//////////////////////////////////////////////////////////////////////////

// Old player sounds - Skid

/////////////////////////////////////////////////////////////////////////////



	//Quakeguy

	gi.soundindex ("idg3/gib.wav");


	//Doomguy

	gi.soundindex ("idg2/push.wav");

	gi.soundindex ("idg2/gib.wav");


	//Wolfie

	gi.soundindex ("idg1/push.wav");

	gi.soundindex ("idg1/gib.wav");




//////////////////////////////////////////////////////////////////////////

	sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	

	//Use for Wolfy

	gi.modelindex ("models/objects/gibs/arm/tris.md2");

	gi.modelindex ("models/objects/gibs/leg/tris.md2");
	gi.modelindex ("models/objects/gibs/bone/tris.md2");
	gi.modelindex ("models/objects/gibs/bone2/tris.md2");
	gi.modelindex ("models/objects/gibs/chest/tris.md2");
	gi.modelindex ("models/objects/idg1head/tris.md2");



	//Quake Gibs



	gi.modelindex ("models/objects/idg3head/tris.md2");

	gi.modelindex ("models/items/idg3powerups/invis/eyes/tris.md2");





	//Doom gibs

	gi.modelindex ("models/objects/idg2gibs/tris.md2");

	gi.modelindex ("models/objects/idg2head/tris.md2");



	gi.modelindex ("sprites/Dtele.sp2"); 


//////////////////////////////////////////////////////////////////////////

//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//

	// 0 normal
	gi.configstring(CS_LIGHTS+0, "m");
	
	// 1 FLICKER (first variety)
	gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
	
	// 2 SLOW STRONG PULSE
	gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
	
	// 3 CANDLE (first variety)
	gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
	
	// 4 FAST STROBE
	gi.configstring(CS_LIGHTS+4, "mamamamamama");
	
	// 5 GENTLE PULSE 1
	gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
	
	// 6 FLICKER (second variety)
	gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
	
	// 7 CANDLE (second variety)
	gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
	
	// 8 CANDLE (third variety)
	gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
	
	// 9 SLOW STROBE (fourth variety)
	gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
	
	// 10 FLUORESCENT FLICKER
	gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
	
	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	gi.configstring(CS_LIGHTS+63, "a");

	// Music stuff

	music_play_song();
}

