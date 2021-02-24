#include "g_local.h"

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;

// GRIM - no monster code
void SP_bullshit (edict_t *self)
{
        self->nextthink = level.time + 1;
        self->think = G_FreeEdict;
	gi.linkentity (self);
}
// GRIM - no monster code

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
void SP_misc_gib_arm (edict_t *self);
void SP_misc_gib_leg (edict_t *self);
void SP_misc_gib_head (edict_t *self);
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

// GRIM
// MONSTER CODE
/*
void SP_misc_actor (edict_t *self);
void SP_misc_insane (edict_t *self);
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
*/
// MONSTER CODE
// GRIM


spawn_t	spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_coop", SP_info_player_coop},
	{"info_player_intermission", SP_info_player_intermission},
  
// GRIM
        {"info_team", SP_info_team},
	{"info_player_team1", SP_info_player_team1},
	{"info_player_team2", SP_info_player_team2},
        {"info_player_team3", SP_info_player_team3},
        {"info_player_team4", SP_info_player_team4},
        {"info_player_team5", SP_info_player_team5},
        {"info_player_team6", SP_info_player_team6},
        {"info_player_team7", SP_info_player_team7},
        {"info_player_team8", SP_info_player_team8},
// GRIM

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

        // Dirty - Goal stuff
        {"target_endgame", SP_target_endgame},
        {"kill_goal", SP_kill_goal},
        {"trigger_goal", SP_trigger_goal},
        {"item_goal", SP_item_goal},
        // Dirty - Goal stuff

	{"target_splash", SP_target_splash},
	{"target_spawner", SP_target_spawner},
	{"target_blaster", SP_target_blaster},
	{"target_crosslevel_trigger", SP_target_crosslevel_trigger},
	{"target_crosslevel_target", SP_target_crosslevel_target},
	{"target_laser", SP_target_laser},
	{"target_help", SP_target_help},
// GRIM
        // MONSTER CODE
        //{"target_actor", SP_target_actor},
        {"target_actor", SP_bullshit},
// GRIM
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

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
// GRIM
        // MONSTER CODE
        //{"misc_actor", SP_misc_actor},
        {"misc_actor", SP_bullshit},
// GRIM
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
// GRIM
        // MONSTER CODE
        //{"misc_insane", SP_misc_insane},
        {"misc_insane", SP_bullshit},
// GRIM
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

// GRIM
        // MONSTER CODE
        /*
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
        */
// MONSTER CODE
        {"monster_berserk", SP_bullshit},
        {"monster_gladiator", SP_bullshit},
        {"monster_gunner", SP_bullshit},
        {"monster_infantry", SP_bullshit},
        {"monster_soldier_light", SP_bullshit},
        {"monster_soldier", SP_bullshit},
        {"monster_soldier_ss", SP_bullshit},
        {"monster_tank", SP_bullshit},
        {"monster_tank_commander", SP_bullshit},
        {"monster_medic", SP_bullshit},
        {"monster_flipper", SP_bullshit},
        {"monster_chick", SP_bullshit},
        {"monster_parasite", SP_bullshit},
        {"monster_flyer", SP_bullshit},
        {"monster_brain", SP_bullshit},
        {"monster_floater", SP_bullshit},
        {"monster_hover", SP_bullshit},
        {"monster_mutant", SP_bullshit},
        {"monster_supertank", SP_bullshit},
        {"monster_boss2", SP_bullshit},
        {"monster_boss3_stand", SP_bullshit},
        {"monster_jorg", SP_bullshit},

        {"monster_commander_body", SP_bullshit},

        {"turret_breach", SP_bullshit},
        {"turret_base", SP_bullshit},
        {"turret_driver", SP_bullshit},
        // MONSTER CODE
// GRIM
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

        if (!strcmp(ent->classname, "info_flag_team1"))
                ent->classname = "item_flag_team1";
        else if (!strcmp(ent->classname, "info_flag_team2"))
                ent->classname = "item_flag_team2";

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
// EntZ 1.01 by Avi Rozen (Zung!)
// e-mail: zungbang@telefragged.com
char current_mapname[MAX_STRING_CHARS];
char *original_entstring = NULL;
char *new_entstring = NULL;
char *added_entstring = NULL;

void Svcmd_EntZ_f (void)
{
	if (original_entstring)
	{
		FILE *entfile;
		char path[MAX_OSPATH];

		// create the folder 
                // Dirty
                //sprintf (path, "dirty/maps/ents");
                //CreateDirectory (path, NULL);
                // Dirty

		// create the file
                // Dirty
                sprintf (path, "dirty/maps/ents/%s.ent", current_mapname);
                // Dirty
		entfile = fopen (path, "wb");
		if (entfile == NULL)
		{
                        gi.dprintf ("can't open \"%s\"\n", path);
			return;
		}
		if (fwrite (original_entstring, strlen (original_entstring) + 1, 1, entfile) != 1)
                        gi.dprintf ("can't write to \"%s\"\n", path);
		else
                        gi.dprintf ("ents saved to \"%s\"\n", path);
		fclose (entfile);
	}
	else
                gi.dprintf ("%s is not a map\n", current_mapname);
}

void RealSpawnEntities (char *mapname, char *entities, char *spawnpoint);

void SpawnThese (char *entities, float skill_level, qboolean not_world)
{
        edict_t *ent;
        char    *com_token;
        int     inhibit;
		//int		i;
		

	ent = NULL;
	inhibit = 0;

	while (1)
	{
		// parse the opening brace	
		com_token = COM_Parse (&entities);
		if (!entities)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);

                if ((!ent) && (!not_world))
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
                        else if (((skill->value == 0) && (ent->spawnflags & SPAWNFLAG_NOT_EASY))
                         || ((skill->value == 1) && (ent->spawnflags & SPAWNFLAG_NOT_MEDIUM))
                          || ((skill->value >= 2) && (ent->spawnflags & SPAWNFLAG_NOT_HARD)))
                        {
                                G_FreeEdict (ent); 
                                inhibit++;
                                continue;
			}

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_NOT_COOP|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}

	gi.dprintf ("%i entities inhibited\n", inhibit);
/*
#ifdef DEBUG
	i = 1;
	ent = EDICT_NUM(i);
	while (i < globals.num_edicts) {
		if (ent->inuse != 0 || ent->inuse != 1)
			Com_DPrintf("Invalid entity %d\n", i);
		i++, ent++;
	}
#endif
*/
}

void SpawnAnts (char *mapname, float skill_level)
{
        if (added_entstring)
	{
                gi.TagFree (added_entstring);
                added_entstring = NULL;
	}

	if (!strstr (mapname, ".")) // is this a map?
	{
		FILE *entfile;
		char path[MAX_OSPATH];

                // load adition entstring from file (and ANT file)
                sprintf (path, "dirty/maps/ents/%s.ant", current_mapname);

		entfile = fopen (path, "rb");
		if (entfile != NULL)
		{
                        gi.dprintf ("loading ants from \"%s\"\n",path);
			if (!fseek (entfile, 0L, SEEK_END))
			{
				long entfile_length;

				entfile_length = ftell (entfile);
				if (entfile_length > 0)
				{
                                        added_entstring = (char *) gi.TagMalloc (entfile_length + 1, TAG_GAME);
                                        if (added_entstring == NULL)
					{
                                                gi.error ("SpawnEntities: can't allocate added_entstring.\n");
						return;
					}
					rewind (entfile);
                                        if (fread (added_entstring, entfile_length, 1, entfile) != 1)
					{
                                                gi.TagFree (added_entstring);
						new_entstring = NULL;
                                                gi.dprintf ("can't read entities\n");
					}
					else
                                                added_entstring[entfile_length] = '\0';
				}
				else
                                        gi.dprintf ("can't determine file size\n");
			}
			else
                                gi.dprintf ("can't find EOF\n");
			fclose (entfile);
		}
	} 

        if (added_entstring)
                SpawnThese (added_entstring, skill_level, true);
}

void SpawnEntities (char *mapname, char *entstring, char *spawnpoint)
{
	strcpy (current_mapname, mapname);
	if (original_entstring)
	{
                gi.TagFree (original_entstring);
		original_entstring = NULL;
	}
	if (new_entstring)
	{
                gi.TagFree (new_entstring);
		new_entstring = NULL;
	}
	if (!strstr (mapname, ".")) // is this a map?
	{
		FILE *entfile;
		char path[MAX_OSPATH];

		// save original entstring for later
                original_entstring = (char *) gi.TagMalloc (strlen (entstring), TAG_GAME);
		if (original_entstring == NULL)
		{
                        gi.error ("SpawnEntities: can't allocate original_entstring.\n");
			return;
		}
		strcpy (original_entstring, entstring);
		// load modified entstring from file
                // Dirty
                sprintf (path, "dirty/maps/ents/%s.ent", current_mapname);
                // Dirty

		entfile = fopen (path, "rb");
		if (entfile != NULL)
		{
                        gi.dprintf ("loading ents from \"%s\"\n",path);
			if (!fseek (entfile, 0L, SEEK_END))
			{
				long entfile_length;

				entfile_length = ftell (entfile);
				if (entfile_length > 0)
				{
                                        new_entstring = (char *) gi.TagMalloc (entfile_length + 1, TAG_GAME);
					if (new_entstring == NULL)
					{
                                                gi.error ("SpawnEntities: can't allocate new_entstring.\n");
						return;
					}
					rewind (entfile);
					if (fread (new_entstring, entfile_length, 1, entfile) != 1)
					{
                                                gi.TagFree (new_entstring);
						new_entstring = NULL;
                                                gi.dprintf ("can't read entities\n");
					}
					else
						new_entstring[entfile_length] = '\0';
				}
				else
                                        gi.dprintf ("can't determine file size\n");
			}
			else
                                gi.dprintf ("can't find EOF\n");
			fclose (entfile);
		}
	} 

        // Dirty
	if (new_entstring)
                RealSpawnEntities (mapname, new_entstring, spawnpoint);
	else
                RealSpawnEntities (mapname, entstring, spawnpoint);
        // Dirty
}

void RealSpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	//edict_t		*ent;
    //int             inhibit;
	//char		*com_token;
	int			i;
	float		skill_level;

        // Dirty - NULL global checks
        max_teams = 0;
        turns_on = false;
        goals_exist = false;
        // Dirty

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

        // Dirty
        SpawnThese (entities, skill_level, false);
        SpawnAnts (mapname, skill_level);
        // Dirty

	G_FindTeams ();

        // Dirty
        if ((!turns_on) && match->value)
                turns_on = true;

        // so give time for players to connect...
        if (turns_on)
        {
                level.match_state = MATCH_CHECKING;
                level.match_time = level.time + 45; // TEST 90;
        }
        else
        {
                level.match_state = MATCH_DURING;
                level.match_time = 0;
        }
        // Dirty

        // Paranoid
        //PlayerTrail_Init ();
        // Paranoid

        // GRIM
        if (ctf->value) // GRIM
        {
                CTFBaseInit(); // Makes bases on non-ctf maps etc
                // Paranoid
                //CTFSetupTechSpawn(); //ZOID
                // Paranoid
        }

        // Dirty
        if (teams->value && (max_teams < 2))
                max_teams = teams->value;
        // Dirty

        SetupSpecialsSpawn();
        // GRIM
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
"yb     -36 "

// GRIM
"xv 50  "
"pic 16 "

"if 18 "
        "xv 50  "
        "pic 18 "
"endif "
"if 19 "
        "xv 50  "
        "pic 19 "
"endif "               
"if 20 "
        "xv 50  "
        "pic 20 "
"endif "
"if 21 "
        "xv 50  "
        "pic 21 "
"endif "
"if 22 "
        "xv 50  "
        "pic 22 "
"endif "
"if 23 "
        "xv 50  "
        "pic 23 "
"endif "

// Armor
"if 4 "
        "xv 50 "
        "pic 4  "
"endif "

"yb	-24 "

// health
"xv 0  "
"hnum  "
//"xv 50 "
//"pic 0 "

// id view state
"if 24 "
        "xv 0   "
        "yb -58 "
        "string2 \"Viewing\" "
        "xv 64  "
        "stat_string 24 "
        "yb -24 "
"endif "
// GRIM

// ammo
"if 2 "
        "xv 246  "
        "num 3 3 "
        //"anum  "
        "xv 296  "
        "pic 2   "
"endif "

// ammo 2 - Other gun etc
"if 28 "
        "yb -50   "
        "xv 246   "
        "num 3 29 "
        "xv 296   "
        "pic 28   "
        "yb -24   "
"endif "

// selected item
"if 6 "
        "xv  90 "
        "pic 6  "
"endif "

// timer
"if 9 "
        "xv 200 "
        "pic 9    "
        "xv 150 "
        "num 3 10 "
"endif "

"yb	-50 "

// picked up item
"if 7 "
        "xv 0   "
        "pic 7  "
        "xv 26  "
        "yb -42 "
        "stat_string 8 "
        "yb -50 "
"endif "

// help / weapon icon
"if 11 "
        "xv 148 "
        "pic 11 "
"endif "

// spectator
"if 27 "
        "xv 0   "
        "yb -58 "
        "string2 \"SPECTATOR MODE\" "
"endif "
;


// GRIM

// GRIM
char *dm_statusbar =
"yb     -36 "

// weight
"xv 50  "
"pic 0  "

// wounds
"if 2 "
        "xv 50  "
        "pic 2  "
"endif "

"if 3 "
        "xv 50  "
        "pic 3  "
"endif "

"if 4 "
        "xv 50  "
        "pic 4  "
"endif "

"if 5 "
        "xv 50  "
        "pic 5  "
"endif "

"if 6 "
        "xv 50  "
        "pic 6  "
"endif "

"if 7 "
        "xv 50  "
        "pic 7  "
"endif "

"if 8 "
        "xv 50  "
        "pic 8  "
"endif "

"yb	-24 "

// health
"xv 0  "
"hnum  "

// Dirty
// large weapon
"if 31 "
        "yb     -76     "
        "xr     -26     "
        "pic    31      "
        "yb     -24     "
"endif "
// Dirty

// ammo 2
"if 11 "
        "yb     -50     "
        "xv     246     "
        "num    3 20    "
        "xv     296     "
        "pic    11      "
        "yb     -24     "
"endif "

// ammo
"if 9 "
        "xv     246     "
        "num    3 10    "
        "xv     296     "
        "pic    9       "
"endif "

"xv     0       "
"yv     0       "

// selected item
"if 16 "
        "yb -24 "
        "xv  90 "
        "pic 16 "
        "xv  0  "
"endif "

"yb	-50 "

// picked up item
"if 17"
        "xv     0       "
        "pic    17      "
        "yb     -42     "
        "xv     26      "
        "stat_string 18 "
        "yb     -50     "
"endif "

// help / weapon icon / progress?
"if 19 "
        "xv 148 "
        "pic 19 "
"endif "

//  frags
"xr -50         "
"yt 2           "
"num 3 14       "
"xr -45         "
"yt 26          "
"string2 \"Frags\"     "

// id view
"if 26 "
        "xv 96 "
        "yb -64 "
        "stat_string 26 "
        "yb -50 "
"endif "

// goal1
"if 21 "
        "yb -48 "
        "xv 192 "
        "pic 21 "
"endif "

// goal2
"if 22 "
        "yb -48 "
        "xv 192 "
        "pic 22 "
"endif "

// zoom / other central things
"if 23 "
        "yv 0   "
        "xv 0   "
        "pic 23 "
"endif "

// extra's
"if 24 "
        "yt 36         "
        "xr -76        "
        "num 3 24       "
"endif "

"if 25 "
        "yt 36         "
        "xr -26         "
        "pic 25         "
"endif "
;

// GRIM
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

        // GRIM - Bullets holes, blood
        //InitOtherQues ();
        // GRIM - Bullets holes, blood

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
//GRIM
        {
                // ScoreBoard icons...
                if (max_teams)
                {
                        gi.imageindex("teams");
                        gi.imageindex("players");
                        gi.imageindex("blank");
                }
                // ID icon...

                gi.soundindex("laugh3.wav");
                gi.soundindex("fight.wav");

                gi.imageindex("g1n");
                gi.imageindex("g1ne");
                gi.imageindex("g1e");
                gi.imageindex("g1se");
                gi.imageindex("g1s");
                gi.imageindex("g1sw");
                gi.imageindex("g1w");
                gi.imageindex("g1nw");
                gi.imageindex("g1none");

                gi.imageindex("g2n");
                gi.imageindex("g2ne");
                gi.imageindex("g2e");
                gi.imageindex("g2se");
                gi.imageindex("g2s");
                gi.imageindex("g2sw");
                gi.imageindex("g2w");
                gi.imageindex("g2nw");
                gi.imageindex("g2none");

                gi.imageindex ("enemy");
		gi.configstring (CS_STATUSBAR, dm_statusbar);
        }
//GRIM
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

        // Paranoid

        // Clear teamgame here, so team#names can replace set ones
        // and replaced ones can be cleared
        memset(&teamgame, 0, sizeof(teamgame));
        memcpy(teamgame.teamlist, teamlist, sizeof(teamlist));
        //gi.dprintf ("st.teams = %i, teams->value = %i\n", st.teams, teams->value);
        // Paranoid

	snd_fry = gi.soundindex ("player/fry.wav");	// standing in lava / slime

        // Dirty
        PrecacheItem (FindItem ("MK23 Pistol"));
        PrecacheItem (FindItem ("Hands"));

        gi.modelindex ("sprites/null.sp2");

        // now for wounds
        gi.imageindex("w_ch_g");
        gi.imageindex("w_ch_y");
        gi.imageindex("w_ch_r");
        gi.imageindex("w_h_g");
        gi.imageindex("w_h_y");
        gi.imageindex("w_h_r");
        gi.imageindex("w_la_g");
        gi.imageindex("w_la_y");
        gi.imageindex("w_la_r");
        gi.imageindex("w_ll_g");
        gi.imageindex("w_ll_y");
        gi.imageindex("w_ll_r");
        gi.imageindex("w_ra_g");
        gi.imageindex("w_ra_y");
        gi.imageindex("w_ra_r");
        gi.imageindex("w_rl_g");
        gi.imageindex("w_rl_y");
        gi.imageindex("w_rl_r");

        // yay...all the progress icons..woohoo :(
        gi.imageindex("p_00");
        gi.imageindex("p_02");
        gi.imageindex("p_04");
        gi.imageindex("p_06");
        gi.imageindex("p_08");
        gi.imageindex("p_10");
        gi.imageindex("p_12");
        gi.imageindex("p_14");
        gi.imageindex("p_16");
        gi.imageindex("p_18");
        gi.imageindex("p_20");
        gi.imageindex("p_22");
        gi.imageindex("p_24");
        gi.imageindex("p_26");
        gi.imageindex("p_28");
        gi.imageindex("p_30");
        gi.imageindex("p_32");
        gi.imageindex("p_34");
        gi.imageindex("p_36");
        gi.imageindex("p_38");
        gi.imageindex("p_40");
        gi.imageindex("p_42");
        gi.imageindex("p_44");
        gi.imageindex("p_46");
        gi.imageindex("p_48");
        gi.imageindex("p_50");
        gi.imageindex("p_52");
        gi.imageindex("p_54");
        gi.imageindex("p_56");
        gi.imageindex("p_58");
        gi.imageindex("p_60");
        gi.imageindex("p_62");
        gi.imageindex("p_64");
        gi.imageindex("p_66");
        gi.imageindex("p_68");
        gi.imageindex("p_70");
        gi.imageindex("p_72");
        gi.imageindex("p_74");
        gi.imageindex("p_76");
        gi.imageindex("p_78");
        gi.imageindex("p_80");
        gi.imageindex("p_82");
        gi.imageindex("p_84");
        gi.imageindex("p_86");
        gi.imageindex("p_88");
        gi.imageindex("p_90");
        gi.imageindex("p_92");
        gi.imageindex("p_94");
        gi.imageindex("p_96");
        gi.imageindex("p_98");
        gi.imageindex("p_full");

        // Great, now do ALL the weight icons too...fuck'n
        gi.imageindex("wt_full");
        gi.imageindex("wt_nowt");
        gi.imageindex("wt_10");
        gi.imageindex("wt_20");
        gi.imageindex("wt_30");
        gi.imageindex("wt_40");
        gi.imageindex("wt_50");
        gi.imageindex("wt_60");
        gi.imageindex("wt_70");
        gi.imageindex("wt_80");
        gi.imageindex("wt_90");
        gi.imageindex("wt_100");
        gi.imageindex("wt_110");
        gi.imageindex("wt_120");
        gi.imageindex("wt_130");
        gi.imageindex("wt_140");
        gi.imageindex("wt_150");
        gi.imageindex("wt_160");
        gi.imageindex("wt_170");
        gi.imageindex("wt_180");
        gi.imageindex("wt_190");
        gi.imageindex("wt_200");
        gi.imageindex("wt_210");
        gi.imageindex("wt_220");
        gi.imageindex("wt_230");
        gi.imageindex("wt_240");
        gi.imageindex("wt_250");
        gi.imageindex("wt_260");
        gi.imageindex("wt_270");
        gi.imageindex("wt_280");
        gi.imageindex("wt_290");
        // Dirty

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
        // Dirty
        gi.modelindex ("#w_akimbo.md2");
        gi.modelindex ("#w_cannon.md2");
        gi.modelindex ("#w_knife.md2");
        gi.modelindex ("#w_m4.md2");
        gi.modelindex ("#w_mk23.md2");
        gi.modelindex ("#w_mp5.md2");
        gi.modelindex ("#w_sniper.md2");
        gi.modelindex ("#w_super90.md2");
        gi.modelindex ("#a_m61frag.md2");
        // Dirty

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

        // Paranoid
        //gi.soundindex ("infantry/inflies1.wav");
        gi.soundindex ("laugh3.wav");
        gi.soundindex ("fight.wav");
        // Paranoid

	sm_meat_index = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	gi.modelindex ("models/objects/gibs/arm/tris.md2");
	gi.modelindex ("models/objects/gibs/bone/tris.md2");
	gi.modelindex ("models/objects/gibs/bone2/tris.md2");
	gi.modelindex ("models/objects/gibs/chest/tris.md2");
	gi.modelindex ("models/objects/gibs/skull/tris.md2");
	gi.modelindex ("models/objects/gibs/head2/tris.md2");

//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//
        // GRIM
        if ((int)bflags->value & BF_LIGHTS_OUT)
        {
                // 0 normal
                gi.configstring(CS_LIGHTS+0, "b");
	
                // 1 FLICKER (first variety)
                gi.configstring(CS_LIGHTS+1, "cbcbcbcbcbcbcccccbcbcbcbcbcbbbbb");
	
                // 2 SLOW STRONG PULSE
                gi.configstring(CS_LIGHTS+2, "aaaaaaaaaaaaaacccccccccccccccc");
	
                // 3 CANDLE (first variety)
                gi.configstring(CS_LIGHTS+3, "b");
	
                // 4 FAST STROBE
                gi.configstring(CS_LIGHTS+4, "abcdcbabcdcbabcdcbabcdcbabcdcba");
	
                // 5 GENTLE PULSE 1
                gi.configstring(CS_LIGHTS+5,"aaaabbbbccccddddccccbbbb");
	
                // 6 FLICKER (second variety)
                gi.configstring(CS_LIGHTS+6, "bbbdbbbaabbbddddaaaabbbcbbb");
	
                // 7 CANDLE (second variety)
                gi.configstring(CS_LIGHTS+7, "b");
	
                // 8 CANDLE (third variety)
                gi.configstring(CS_LIGHTS+8, "a");
	
                // 9 SLOW STROBE (fourth variety)
                gi.configstring(CS_LIGHTS+9, "aaaabbbcccdddeeefffeeedddcccbbbaaa");
	
                // 10 FLUORESCENT FLICKER
                gi.configstring(CS_LIGHTS+10, "aagbbbbbbbbbbb");

                // 11 SLOW PULSE NOT FADE TO BLACK
                gi.configstring(CS_LIGHTS+11, "b");
	
                // styles 32-62 are assigned by the light program for switchable lights

                // 63 testing
                gi.configstring(CS_LIGHTS+63, "b");
        }
        else
        {
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
        }
        // GRIM
}

