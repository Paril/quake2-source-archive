#include "g_local.h"

#ifdef WESQ2
int				NumSpawnedItems;
SPAWNED_ITEM	SpawnedItem[MAX_SPAWNED_ITEMS];
#endif

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

// Lazarus
void SP_crane_beam (edict_t *self);
void SP_crane_hoist (edict_t *self);
void SP_crane_hook (edict_t *self);
void SP_crane_control (edict_t *self);
void SP_crane_reset (edict_t *self);
void SP_hint_path (edict_t *self);
void SP_func_bobbingwater (edict_t *self);
void SP_func_door_rot_dh (edict_t *self);
void SP_func_door_swinging (edict_t *self);
void SP_func_force_wall(edict_t *ent);
void SP_func_monitor (edict_t *self);
void SP_func_pendulum (edict_t *self);
void SP_func_pivot (edict_t *self);
void SP_func_pushable (edict_t *self);
void SP_func_reflect (edict_t *self);
void SP_func_rotating_dh (edict_t *self);
void SP_func_trackchange (edict_t *self);
void SP_func_tracktrain (edict_t *self);
void SP_func_trainbutton (edict_t *self);
void SP_func_vehicle (edict_t *self);
void SP_info_train_start (edict_t *self);
void SP_misc_light (edict_t *self);
void SP_model_spawn (edict_t *self);
void SP_model_train (edict_t *self);
void SP_model_turret (edict_t *self);
void SP_monster_makron (edict_t *self);
void SP_path_track (edict_t *self);
void SP_target_anger (edict_t *self);
void SP_target_animation (edict_t *self);
void SP_target_attractor (edict_t *self);
void SP_target_CD (edict_t *self);
void SP_target_change (edict_t *self);
void SP_target_clone (edict_t *self);
void SP_target_effect (edict_t *self);
void SP_target_fade (edict_t *self);
void SP_target_failure (edict_t *self);
void SP_target_fog (edict_t *self);
void SP_target_fountain (edict_t *self);
void SP_target_lightswitch (edict_t *self);
void SP_target_locator (edict_t *self);
void SP_target_lock (edict_t *self);
void SP_target_lock_clue (edict_t *self);
void SP_target_lock_code (edict_t *self);
void SP_target_lock_digit (edict_t *self);
void SP_target_monitor (edict_t *ent);
void SP_target_monsterbattle (edict_t *self);
void SP_target_movewith (edict_t *self);
void SP_target_precipitation (edict_t *self);
void SP_target_rocks (edict_t *self);
void SP_target_rotation (edict_t *self);
void SP_target_set_effect (edict_t *self);
void SP_target_skill (edict_t *self);
void SP_target_sky (edict_t *self);
void SP_target_playback (edict_t *self);
void SP_target_text (edict_t *self);
void SP_thing (edict_t *self);
void SP_tremor_trigger_multiple (edict_t *self);
void SP_trigger_bbox (edict_t *self);
void SP_trigger_disguise (edict_t *self);
void SP_trigger_fog (edict_t *self);
void SP_trigger_inside (edict_t *self);
void SP_trigger_look (edict_t *self);
void SP_trigger_mass (edict_t *self);
void SP_trigger_scales (edict_t *self);
void SP_trigger_speaker (edict_t *self);
void SP_trigger_switch (edict_t *self);
void SP_trigger_teleporter (edict_t *self);
void SP_trigger_transition (edict_t *self);
// transition entities
void SP_bolt (edict_t *self);
void SP_debris (edict_t *self);
void SP_gib (edict_t *self);
void SP_gibhead (edict_t *self);
void SP_grenade (edict_t *self);
void SP_handgrenade (edict_t *self);
void SP_rocket (edict_t *self);
//
// end Lazarus

#ifdef WESQ2
void SP_misc_bomb (edict_t *self);
void SP_misc_ladder (edict_t *self);
void SP_misc_tank1 (edict_t *self);
void SP_misc_tank2 (edict_t *self);
#endif

spawn_t	spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},

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
	{"func_reflect", SP_func_reflect},
	{"func_wall", SP_func_wall},
	{"func_object", SP_func_object},
	{"func_timer", SP_func_timer},
	{"func_explosive", SP_func_explosive},
	{"func_killbox", SP_func_killbox},

	{"target_actor", SP_target_actor},
	{"target_animation", SP_target_animation},
	{"target_blaster", SP_target_blaster},
	{"target_changelevel", SP_target_changelevel},
	{"target_character", SP_target_character},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_earthquake", SP_target_earthquake},
	{"target_explosion", SP_target_explosion},
	{"target_goal", SP_target_goal},
	{"target_help", SP_target_help},
	{"target_laser", SP_target_laser},
	{"target_lightramp", SP_target_lightramp},
	{"target_secret", SP_target_secret},
	{"target_spawner", SP_target_spawner},
	{"target_speaker", SP_target_speaker},
	{"target_splash", SP_target_splash},
	{"target_string", SP_target_string},
	{"target_temp_entity", SP_target_temp_entity},

	{"trigger_always", SP_trigger_always},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_key", SP_trigger_key},
	{"trigger_once", SP_trigger_once},
	{"trigger_monsterjump", SP_trigger_monsterjump},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_push", SP_trigger_push},
	{"trigger_relay", SP_trigger_relay},

	{"viewthing", SP_viewthing},
	{"worldspawn", SP_worldspawn},

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

	{"monster_commander_body", SP_monster_commander_body},

	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_driver", SP_turret_driver},

// Lazarus
	{"crane_beam",   SP_crane_beam},
	{"crane_hoist",  SP_crane_hoist},
	{"crane_hook",   SP_crane_hook},
	{"crane_control",SP_crane_control},
	{"crane_reset",SP_crane_reset},
	{"func_bobbingwater", SP_func_bobbingwater},
	{"func_door_rot_dh", SP_func_door_rot_dh},
	{"func_door_swinging", SP_func_door_swinging},
	{"func_force_wall", SP_func_force_wall},
	{"func_monitor", SP_func_monitor},
	{"func_pendulum", SP_func_pendulum},
	{"func_pivot", SP_func_pivot},
	{"func_pushable", SP_func_pushable},
	{"func_rotating_dh", SP_func_rotating_dh},
	{"func_trackchange", SP_func_trackchange},
	{"func_tracktrain", SP_func_tracktrain},
	{"func_trainbutton", SP_func_trainbutton},
	{"func_vehicle", SP_func_vehicle},
	{"hint_path", SP_hint_path},
	{"info_train_start", SP_info_train_start},
	{"misc_light", SP_misc_light},
	{"model_spawn",  SP_model_spawn},
	{"model_train", SP_model_train},
	{"model_turret", SP_model_turret},
	{"monster_makron", SP_monster_makron},
	{"path_track", SP_path_track},
	{"target_anger", SP_target_anger},
	{"target_attractor", SP_target_attractor},
	{"target_bmodel_spawner", SP_target_clone},
	{"target_cd", SP_target_CD},
	{"target_change", SP_target_change},
	{"target_clone", SP_target_clone},
	{"target_effect", SP_target_effect},
	{"target_fade", SP_target_fade},
	{"target_failure", SP_target_failure},
	{"target_fog", SP_target_fog},
	{"target_fountain", SP_target_fountain},
	{"target_lightswitch", SP_target_lightswitch},
	{"target_locator", SP_target_locator},
	{"target_lock", SP_target_lock},
	{"target_lock_clue", SP_target_lock_clue},
	{"target_lock_code", SP_target_lock_code},
	{"target_lock_digit", SP_target_lock_digit},
	{"target_monitor", SP_target_monitor},
	{"target_monsterbattle", SP_target_monsterbattle},
	{"target_movewith", SP_target_movewith},
	{"target_precipitation", SP_target_precipitation},
	{"target_rocks", SP_target_rocks},
	{"target_rotation", SP_target_rotation},
	{"target_set_effect", SP_target_set_effect},
	{"target_skill", SP_target_skill},
	{"target_sky", SP_target_sky},
	{"target_playback", SP_target_playback},
	{"target_text", SP_target_text},
	{"thing", SP_thing},
	{"tremor_trigger_multiple", SP_tremor_trigger_multiple},
	{"trigger_bbox", SP_trigger_bbox},
	{"trigger_disguise", SP_trigger_disguise},
	{"trigger_fog", SP_trigger_fog},
	{"trigger_inside", SP_trigger_inside},
	{"trigger_look", SP_trigger_look},
	{"trigger_mass", SP_trigger_mass},
	{"trigger_scales", SP_trigger_scales},
	{"trigger_speaker", SP_trigger_speaker},
	{"trigger_switch", SP_trigger_switch},
	{"trigger_teleporter", SP_trigger_teleporter},
	{"trigger_transition", SP_trigger_transition},
// transition entities
	{"bolt", SP_bolt},
	{"debris", SP_debris},
	{"gib", SP_gib},
	{"gibhead", SP_gibhead},
	{"grenade", SP_grenade},
	{"hgrenade", SP_handgrenade},
	{"rocket", SP_rocket},
	{"homing rocket", SP_rocket},
// end Lazarus

#ifdef WESQ2
	{"misc_bomb", SP_misc_bomb},
	{"misc_ladder", SP_misc_ladder},
	{"misc_tank1", SP_misc_tank1},
	{"misc_tank2", SP_misc_tank2},
#endif

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

	// Lazarus: if this fails, edict is freed.

	if (!ent->classname)
	{
		gi.dprintf ("ED_CallSpawn: NULL classname\n");
		G_FreeEdict(ent);
		return;
	}

	// Lazarus: Preserve original angles for movewith stuff 
	//          before G_SetMoveDir wipes 'em out
	VectorCopy(ent->s.angles, ent->org_angles);

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
	G_FreeEdict(ent);
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
		// Lazarus: some entities may have psuedo-teams that shouldn't be handled here
		if (e->class_id == ENTITY_TARGET_CHANGE)
			continue;
		if (e->class_id == ENTITY_TARGET_CLONE)
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

	if(level.time < 2)
		gi.dprintf ("%i teams with %i entities\n", c, c2);
}

void trans_ent_filename (char *);
void ReadEdict (FILE *f, edict_t *ent);
void LoadTransitionEnts()
{
	if(developer->value)
		gi.dprintf("==== LoadTransitionEnts ====\n");
	if(game.transition_ents)
	{
		char		t_file[_MAX_PATH];
		int			i, j;
		FILE		*f;
		vec3_t		v_spawn;
		edict_t		*ent;
		edict_t		*spawn;
		
		VectorClear(v_spawn);
		if(strlen(game.spawnpoint))
		{
			spawn = G_Find(NULL,FOFS(targetname),game.spawnpoint);
			while(spawn)
			{
				if(spawn->class_id == ENTITY_INFO_PLAYER_START)
				{
					VectorCopy(spawn->s.origin,v_spawn);
					break;
				}
				spawn = G_Find(spawn,FOFS(targetname),game.spawnpoint);
			}
		}
		trans_ent_filename (t_file);
		f = fopen(t_file,"rb");
		if(!f)
			gi.error("LoadTransitionEnts: Cannot open %s\n",t_file);
		else
		{
			for(i=0; i<game.transition_ents; i++)
			{
				ent = G_Spawn();
				ReadEdict(f,ent);
				// Correction for monsters with health EXACTLY 0
				// If we don't do this, spawn function will bring
				// 'em back to life
				if(ent->svflags & SVF_MONSTER)
				{
					if(!ent->health)
					{
						ent->health = -1;
						ent->deadflag = DEAD_DEAD;
					}
					else if(ent->deadflag == DEAD_DEAD)
					{
						ent->health = min(ent->health,-1);
					}
				}
				VectorAdd(ent->s.origin,v_spawn,ent->s.origin);
				VectorCopy(ent->s.origin,ent->s.old_origin);
				ED_CallSpawn (ent);
				if(ent->owner_id)
				{
					if(ent->owner_id < 0)
					{
						ent->owner = &g_edicts[-ent->owner_id];
					}
					else
					{
						// We KNOW owners precede owned ents in the 
						// list because of the way it was constructed
						ent->owner = NULL;
						for(j=game.maxclients+1; j<globals.num_edicts && !ent->owner; j++)
						{
							if(ent->owner_id == g_edicts[j].id)
								ent->owner = &g_edicts[j];
						}
					}
					ent->owner_id = 0;
				}
				ent->s.renderfx |= RF_IR_VISIBLE;
			}
			fclose(f);
		}
	}
}

#ifdef WESQ2
/*
==============
SpawnEntfileItems
==============
*/
void SpawnEntfileItems()
{
	void GetEntFilename(char *);
	char	filename[256];
	FILE	*f;
	int		result;
	edict_t	*spawn;

	GetEntFilename(filename);
	f = fopen(filename,"r");
	if(!f) return;
	NumSpawnedItems = 0;
	while (1)
	{
		result = fscanf(f,"%s %f %f %f %f",
			SpawnedItem[NumSpawnedItems].classname,
			&SpawnedItem[NumSpawnedItems].origin[0],
			&SpawnedItem[NumSpawnedItems].origin[1],
			&SpawnedItem[NumSpawnedItems].origin[2],
			&SpawnedItem[NumSpawnedItems].angle     );
		if(result != 5) break;
		spawn = G_Spawn();
		spawn->classname = gi.TagMalloc((strlen(SpawnedItem[NumSpawnedItems].classname)+1)*sizeof(char), TAG_LEVEL);
		spawn->classname = SpawnedItem[NumSpawnedItems].classname;
		VectorCopy(SpawnedItem[NumSpawnedItems].origin,spawn->s.origin);
		spawn->s.angles[1] = SpawnedItem[NumSpawnedItems].angle;
		ED_CallSpawn (spawn);
		gi.unlinkentity (spawn);
		KillBox (spawn);
		gi.linkentity (spawn);
		spawn->s.renderfx |= RF_IR_VISIBLE;
		spawn->my_spawn = NumSpawnedItems+1;
		NumSpawnedItems++;
		if(NumSpawnedItems >= MAX_SPAWNED_ITEMS) break;
	}
	fclose(f);
}
/*
=================
ReadPeakPressures

Reads <mapname>.prs if it exists for peak pressure/temperature info and
charge locations
=================*/
void ReadPeakPressures()
{

	cvar_t	*game;
	FILE	*f;
	char	filename[256];
	int		i;

	game = gi.cvar("game", "", CVAR_SERVERINFO| CVAR_LATCH);
	strcpy(filename,game->string);
	strcat(filename,"/maps/");
	strcat(filename,level.mapname);
	strcat(filename,".prs");
	f = fopen(filename,"rt");
	if(!f) return;
	gNumCharges = 0;
	gNumTargets = 0;
	gTargetSpacing = 0;
	if(PT) free(PT);
	if(TNT) free(TNT);
	fscanf(f,"%f %f %f",&gWorld[0],&gWorld[1],&gWorld[2]);
	fscanf(f,"%d",&gNumCharges);
	if(gNumCharges) {
		edict_t	*bomb;
		TNT = (EXPLOSIVE *)calloc(gNumCharges,sizeof(EXPLOSIVE));
		for(i=0; i<gNumCharges; i++) {
			fscanf(f,"%f %f %f %f %f",&TNT[i].loc[0],&TNT[i].loc[1],&TNT[i].loc[2],
				&TNT[i].weight,&TNT[i].delay);
			bomb = G_Spawn();
			bomb->classname = gi.TagMalloc((strlen("misc_bomb")+1)*sizeof(char), TAG_LEVEL);
			bomb->classname = "misc_bomb";
			VectorCopy(TNT[i].loc,bomb->s.origin);
			ED_CallSpawn (bomb);
			gi.unlinkentity (bomb);
			KillBox (bomb);
			gi.linkentity (bomb);
			bomb->s.renderfx |= RF_IR_VISIBLE;
			bomb->mass = (int)(TNT[i].weight);
			bomb->dmg  = (int)(TNT[i].weight*DAMAGE_PER_POUND);
		}
	}
	fscanf(f,"%f",&gTargetSpacing);
	fscanf(f,"%d",&gNumTargets);
	if(gNumTargets) {
		PT = (PRESSURE_TEMP *)calloc(gNumTargets,sizeof(PRESSURE_TEMP));
		for(i=0; i<gNumTargets; i++) {
			fscanf(f,"%f %f %f %f %f",&PT[i].loc[0],&PT[i].loc[1],&PT[i].loc[2],
				&PT[i].pressure,&PT[i].temperature);
		}
	}
	fclose(f);
}
#endif
/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;
	extern int	max_modelindex;
	extern int	max_soundindex;
	extern int	lastgibframe;

	if(developer->value)
		gi.dprintf("====== SpawnEntities ========\n");
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
	// Lazarus: these are used to track model and sound indices
	//          in g_main.c:
	max_modelindex = 0;
	max_soundindex = 0;

	// Lazarus: last frame a gib was spawned in
	lastgibframe = 0;

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);

	// set client fields on player ents
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;

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
				if (((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY)) ||
					((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM)) ||
					(((skill->value == 2) || (skill->value == 3)) && (ent->spawnflags & SPAWNFLAG_NOT_HARD))
					)
					{
						G_FreeEdict (ent);	
						inhibit++;
						continue;
					}
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
		ent->s.renderfx |= RF_IR_VISIBLE;		//PGM
	}	

#ifdef WESQ2
	SpawnEntfileItems();
	ReadPeakPressures();
#endif

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

	// DWH
	G_FindCraneParts();

	// Get origin offsets (mainly for brush models w/o origin brushes)
	for (i=1, ent=g_edicts+i ; i < globals.num_edicts ; i++,ent++)
	{
		VectorAdd(ent->absmin,ent->absmax,ent->origin_offset);
		VectorScale(ent->origin_offset,0.5,ent->origin_offset);
		VectorSubtract(ent->origin_offset,ent->s.origin,ent->origin_offset);
	}

	// end DWH

	PlayerTrail_Init ();

	if(!deathmatch->value)
		InitHintPaths();

	for(i=1, ent=g_edicts+i; i < globals.num_edicts; i++, ent++)
	{
		if(!ent->movewith)
			continue;
		if(ent->movewith_ent)
			continue;
		ent->movewith_ent = G_Find(NULL,FOFS(targetname),ent->movewith);
		// Make sure that we can really "movewith" this guy. This check
		// allows us to have movewith parent with same targetname as
		// other entities
		while(ent->movewith_ent &&
			( (ent->movewith_ent->class_id != ENTITY_FUNC_TRAIN)     &&
			  (ent->movewith_ent->class_id != ENTITY_MODEL_TRAIN)    &&
			  (ent->movewith_ent->class_id != ENTITY_FUNC_DOOR)      &&
			  (ent->movewith_ent->class_id != ENTITY_FUNC_VEHICLE)   &&
			  (ent->movewith_ent->class_id != ENTITY_FUNC_TRACKTRAIN)   ) )
			 ent->movewith_ent = G_Find(ent->movewith_ent,FOFS(targetname),ent->movewith);
		if(ent->movewith_ent)
			movewith_init(ent->movewith_ent);
	}

/*	for(i=1, ent=g_edicts+i; i < globals.num_edicts; i++, ent++)
	{
		gi.dprintf("%s:%s - movewith=%s, movewith_ent=%s:%s, movewith_next=%s:%s\n====================\n",
			ent->classname, (ent->targetname ? ent->targetname : "noname"),
			(ent->movewith ? ent->movewith : "N/A"),
			(ent->movewith_ent ? ent->movewith_ent->classname : "N/A"),
			(ent->movewith_ent ? (ent->movewith_ent->targetname ? ent->movewith_ent->targetname : "noname") : "N/A"),
			(ent->movewith_next ? ent->movewith_next->classname : "N/A"),
			(ent->movewith_next ? (ent->movewith_next->targetname ? ent->movewith_next->targetname : "noname") : "N/A"));

	} */

	if(game.transition_ents)
		LoadTransitionEnts();

	actor_files();

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

#ifdef WESQ2

char *single_statusbar = 
"yb	-24 "

// pressure
"if 18 "
"   xv 0 "
"	num 5 19 "
"	xv	90 "
"   pic 18 "
"endif "

// temperature
"if 20 "
"   xv 200 "
"	num 5 21 "
"	xv 290 "
"   pic 20 "
"endif "

// vehicle speed
"if 22 "
"	yb -90 "
"	xv 128 "
"	pic 22 "
"endif "

;

char *dm_statusbar = 
"yb	-24 "

// pressure
"if 18 "
"	xv	80 "
"   pic 18 "
"   xv 106 "
"	num 5 19 "
"endif "

// temperature
"if 20 "
"	xv	220 "
"   pic 20 "
"   xv 246 "
"	num 5 21 "
"endif "

// vehicle speed
"if 22 "
"	yb -90 "
"	xv 128 "
"	pic 22 "
"endif "
;

#else

char *single_statusbar = 
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

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
"if 6 "
"	xv	296 "
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

// timer (was xv 262)
"if 9 "
"	xv	230 "
"	num	4 10 "
"	xv	296 "
"	pic	9 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "

// vehicle speed
"if 22 "
"	yb -90 "
"	xv 128 "
"	pic 22 "
"endif "

// zoom
"if 23 "
"   yv 0 "
"   xv 0 "
"   pic 23 "
"endif "
;


char *dm_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

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
"if 6 "
"	xv	296 "
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
"	xv	230 "
"	num	4 10 "
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
"num 3 14 "

// spectator
"if 17 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "

// vehicle speed
"if 22 "
"	yb -90 "
"	xv 128 "
"	pic 22 "
"endif "
;

#endif	// ifdef WESQ2

/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
//void SetChromakey();
void SP_worldspawn (edict_t *ent)
{
	ent->class_id = ENTITY_WORLDSPAWN;
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

	if (st.sky && st.sky[0])
		gi.configstring (CS_SKY, st.sky);
	else
		gi.configstring (CS_SKY, "unit1_");

	gi.configstring (CS_SKYROTATE, va("%f", st.skyrotate) );

	gi.configstring (CS_SKYAXIS, va("%f %f %f",
		st.skyaxis[0], st.skyaxis[1], st.skyaxis[2]) );

	gi.configstring (CS_CDTRACK, va("%i", ent->sounds) );

	gi.configstring (CS_MAXCLIENTS, va("%i", (int)(maxclients->value) ) );

	// status bar program
	if (deathmatch->value)
		gi.configstring (CS_STATUSBAR, dm_statusbar);
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

	sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex ("models/objects/gibs/arm/tris.md2");
	gi.modelindex ("models/objects/gibs/bone/tris.md2");
	gi.modelindex ("models/objects/gibs/bone2/tris.md2");
	gi.modelindex ("models/objects/gibs/chest/tris.md2");
	gi.modelindex ("models/objects/gibs/skull/tris.md2");
	gi.modelindex ("models/objects/gibs/head2/tris.md2");

	gi.soundindex ("mud/mud_in2.wav");
	gi.soundindex ("mud/mud_out1.wav");
	gi.soundindex ("mud/mud_un1.wav");
	gi.soundindex ("mud/wade_mud1.wav");
	gi.soundindex ("mud/wade_mud2.wav");

	Lights();

	// Fog clipping - if "fogclip" is non-zero, force gl_clear to a good
	// value for obscuring HOM with fog... "good" is driver-dependent
	if(ent->fogclip)
	{
		if(gl_driver && !Q_stricmp(gl_driver->string,"3dfxgl"))
			gi.cvar_forceset("gl_clear", "0");
		else
			gi.cvar_forceset("gl_clear", "1");
	}

	// FMOD 3D sound attenuation:
	if(ent->attenuation <= 0.)
		ent->attenuation = 1.0;

	// FMOD 3D sound Doppler shift:
	if(st.shift > 0)
		ent->moveinfo.distance = st.shift;
	else if(st.shift < 0)
		ent->moveinfo.distance = 0.0;
	else
		ent->moveinfo.distance = 1.0;

	// cvar overrides for effects flags:
	if(alert_sounds->value)
		world->effects |= FX_WORLDSPAWN_ALERTSOUNDS;
	if(corpse_fade->value)
		world->effects |= FX_WORLDSPAWN_CORPSEFADE;
	if(jump_kick->value)
		world->effects |= FX_WORLDSPAWN_JUMPKICK;
	if(footstep_sounds->value)
		world->effects |= FX_WORLDSPAWN_STEPSOUNDS;

	if(deathmatch->value || coop->value)
		qFMOD_Footsteps = false;
	else if(world->effects & FX_WORLDSPAWN_STEPSOUNDS)
	{
		qFMOD_Footsteps = true;
		FMOD_Init();
	}
	else
		qFMOD_Footsteps = false;
}

// Hud toggle ripped from TPP source

int nohud = 0;

void Hud_On()
{
	if (deathmatch->value)
		gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);
	nohud = 0;
}

void Hud_Off()
{
	gi.configstring (CS_STATUSBAR, NULL);
	nohud = 1;
}

void Cmd_ToggleHud ()
{
	if (deathmatch->value)
		return;
  if (nohud)
		Hud_On();
	else
		Hud_Off();
}
