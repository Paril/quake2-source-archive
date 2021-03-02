
#include "g_local.h"

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;

void SP_item_health (edict_t *self);
void SP_item_health_small (edict_t *self);
void SP_item_health_large (edict_t *self);
void SP_item_health_mega (edict_t *self);

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

void SP_trigger_fog (edict_t *self);
void SP_trigger_smooth_fog (edict_t *ent);

void SP_effect_lightning (edict_t *self);
void SP_func_computer (edict_t *ent);
void SP_func_coolgrav (edict_t *ent);
void SP_SpriteFountain (edict_t *ent);

void SP_monster_bird (edict_t *self);
void SP_monster_jawa (edict_t *self);
void SP_monster_jawa2 (edict_t *self);
void SP_monster_jawa3 (edict_t *self);
void SP_monster_officer (edict_t *self);
void SP_monster_r2 (edict_t *self);
void SP_monster_ske27 (edict_t *self);
void SP_monster_trooper (edict_t *ent);

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
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

//ZOID
	{"info_player_team1", SP_info_player_team1},
	{"info_player_team2", SP_info_player_team2},
//ZOID
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
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
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

//CaRRaC
//	{"misc_skimmer", SP_misc_skimmer},
	{"misc_hologram", SP_misc_hologram},
	{"misc_rain", SP_misc_rain},
	{"misc_object", SP_misc_object},
	{"misc_flame", SP_misc_flame},
	{"misc_setlights", SP_misc_setlights},
	{"misc_ship", SP_misc_flyby},
	{"misc_flyby", SP_misc_flyby},
	{"misc_highlight", SP_misc_highlight},
	{"misc_bigbolt", SP_misc_bigbolt},
	{"misc_rubble", SP_misc_rubble},

	{"trigger_fog", SP_trigger_fog},
	{"trigger_smooth_fog", SP_trigger_smooth_fog},

	{"effect_lightning", SP_effect_lightning},
	{"func_computer", SP_func_computer},
	{"func_coolgrav", SP_func_coolgrav},
	{"func_fountain", SP_SpriteFountain},

	{"monster_bird", SP_monster_bird},
	{"monster_jawa", SP_monster_jawa},
	{"monster_jawa_worker", SP_monster_jawa2},
	{"monster_jawa_trader", SP_monster_jawa3},
	{"monster_officer", SP_monster_officer},
	{"monster_r2", SP_monster_r2},
	{"monster_ske27", SP_monster_ske27},
	{"monster_trooper", SP_monster_trooper},

	{"monster_soldier", SP_monster_trooper},
	{"monster_soldier_light", SP_monster_trooper},
	{"monster_soldier_ss", SP_monster_trooper},
	{"monster_infantry", SP_monster_trooper},

	{NULL, NULL}
};

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void SWTC_Fix_Quake2_Items(edict_t *ent)
{
	if(!strcmp("weapon_shotgun", ent->classname))
	{
		if(deathmatch->value)
			ent->classname = "weapon_blasterrifle";
	}
	else if(!strcmp("weapon_supershotgun", ent->classname))
	{
		ent->classname = "weapon_bowcaster";
	}
	else if(!strcmp("weapon_machinegun", ent->classname))
	{
		ent->classname = "weapon_repeater";
	}
	else if(!strcmp("weapon_chaingun", ent->classname))
	{
		ent->classname = "weapon_wristrocket";
	}
	else if(!strcmp("ammo_grenades", ent->classname))
	{
		ent->classname = "ammo_thermal";
	}
	else if(!strcmp("weapon_grenadelauncher", ent->classname))
	{
		ent->classname = "ammo_thermal";
	}
	else if(!strcmp("weapon_rocketlauncher", ent->classname))
	{
		ent->classname = "weapon_missiletube";
	}
	else if(!strcmp("weapon_hyperblaster", ent->classname))
	{
		ent->classname = "weapon_disruptor";
	}
	else if(!strcmp("weapon_railgun", ent->classname))
	{
		ent->classname = "weapon_sniper";
	}
	else if(!strcmp("weapon_nightstinger", ent->classname))
	{
		ent->classname = "weapon_sniper";
	}
	else if(!strcmp("weapon_bfg", ent->classname))
	{
		ent->classname = "weapon_beamtube";
	}
	else if(!strcmp("ammo_shells", ent->classname))
	{
		if(random() > 0.5)
			ent->classname = "ammo_e11";
		else
			ent->classname = "ammo_dl44";
	}
	else if(!strcmp("ammo_bullets", ent->classname))
	{
		if(random() > 0.5)
			ent->classname = "ammo_t21";
		else
			ent->classname = "ammo_bolts";
	}
	else if(!strcmp("ammo_cells", ent->classname))
	{
//		if(random() > 0.5)
//			ent->classname = "ammo_dis";
//		else
			ent->classname = "ammo_bpack";
	}
	else if(!strcmp("ammo_rockets", ent->classname))
	{
		if(random() > 0.5)
			ent->classname = "ammo_cmiss";
		else
			ent->classname = "ammo_bmiss";
	}
	else if(!strcmp("item_quad", ent->classname))
			ent->classname = "item_armor_jacket";
	else if(!strcmp("item_adrenaline", ent->classname))
			ent->classname = "item_armor_jacket";
	else if(!strcmp("item_power_shield", ent->classname))
			ent->classname = "item_armor_jacket";
	else if(!strcmp("misc_ctf_small_banner", ent->classname))
			ent->classname = "item_armor_jacket";
	else if(!strcmp("item_pack", ent->classname))
			ent->classname = "item_armor_jacket";
	else if(!strcmp("item_bandolier", ent->classname))
			ent->classname = "item_armor_jacket";
	else if(!strcmp("ammo_slugs", ent->classname))
	{
		ent->classname = "ammo_sniper";
	}
}

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

	SWTC_Fix_Quake2_Items(ent);
	
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
			if (deathmatch->value && !ctf->value)
			{
				if ( ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
				{
					G_FreeEdict (ent);	
					inhibit++;
					continue;
				}
			}
			else if (ctf->value && ent->spawnflags & SPAWNFLAG_NOT_COOP)
			{
				G_FreeEdict (ent);	
				inhibit++;
				continue;
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
"pic 0 "

// clipammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// ammo
"if 4 "
"	xv	174 "
"	num 2 4 "
"endif "

// selected item
"if 6 "
"	xv	288 "
"	pic 6 "
"endif "

//  Force Menu bar
"if 11 "
"	yt	12 "
"	xl	48 "
"	pic	11 "
"endif "

//  Force Power 1
"if 20 "
"	yt	44 "
"	xl	48 "
"	pic	20 "
"endif "

//  Force Power 2
"if 21 "
"	yt	76 "
"	xl	48 "
"	pic	21 "
"endif "

//  Force Power 3
"if 22 "
"	yt	108 "
"	xl	48 "
"	pic	22 "
"endif "

//  Force Power 4
"if 23 "
"	yt	140 "
"	xl	48 "
"	pic	23 "
"endif "

//  Force Power 5
"if 24 "
"	yt	172 "
"	xl	48 "
"	pic	24 "
"endif "

"	xl	0 "

// active power
"if 16 "
"	yb	-50 "
"	pic 16 "
"endif "

"if 17 "
"	yb	-74 "
"	pic 17 "
"endif "

"if 18 "
"	yb	-98 "
"	pic 18 "
"endif "

"if 19 "
"	yb	-122 "
"	pic 19 "
"endif "

//sniper
"if 7 "
"	xv	0 "
"	yv	0 "
"	pic	7 "
"	xv	26 "
"	yb	-42 "
"	string \"Zoomed\" "
"	yb	-50 "
"endif "

// current force power
"if 8 "
"	yb	-50 "
"	xr	-24 "
"	pic 8 "
"endif "

// force pool
"if 9 "
"	xr	-72 "
"	num 3 9"
"endif "
;

char *dm_statusbar = 
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// clipammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// ammo
"if 4 "
"	xv	174 "
"	num 2 4 "
"endif "

// selected item
"if 6 "
"	xv	288 "
"	pic 6 "
"endif "

"yb	-50 "
"	xv	4 "

//  Force Menu bar
"if 11 "
"	yt	12 "
"	xl	48 "
"	pic	11 "
"endif "

//  Force Power 1
"if 20 "
"	yt	44 "
"	xl	48 "
"	pic	20 "
"endif "

//  Force Power 2
"if 21 "
"	yt	76 "
"	xl	48 "
"	pic	21 "
"endif "

//  Force Power 3
"if 22 "
"	yt	108 "
"	xl	48 "
"	pic	22 "
"endif "

//  Force Power 4
"if 23 "
"	yt	140 "
"	xl	48 "
"	pic	23 "
"endif "

//  Force Power 5
"if 24 "
"	yt	172 "
"	xl	48 "
"	pic	24 "
"endif "

"	xl	0 "

// active power
"if 16 "
"	yb	-50 "
"	pic 16 "
"endif "

"if 17 "
"	yb	-74 "
"	pic 17 "
"endif "

"if 18 "
"	yb	-98 "
"	pic 18 "
"endif "

"if 19 "
"	yb	-122 "
"	pic 19 "
"endif "

"if 7 "
"	xv	0 "
"	yv	0 "
"	pic	7 "
"endif "

// id view state
"if 27 "
  "xv 0 "
  "yb -58 "
  "string \"Viewing\" "
  "xv 64 "
  "stat_string 27 "
"endif "

// current force power
"if 8 "
"	yb	-50 "
"	xr	-24 "
"	pic 8 "
"endif "

// force pool
"if 9 "
"	xr	-72 "
"	num 3 9"
"endif "

//  frags
"xr	-50 "
"yt 2 "
"num 3 14 "
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
	if(ctf->value)
		gi.configstring (CS_STATUSBAR, ctf_statusbar);
	else if (deathmatch->value)
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
	PrecacheItem (FindItem ("Lightsaber"));

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
	gi.modelindex ("#w_pist.md2");		//blaster pistol
	gi.modelindex ("#w_trpr.md2");		//blaster rifle
	gi.modelindex ("#w_rapid.md2");		//repeater
	gi.modelindex ("#w_arrw.md2");		//bowcaster
	gi.modelindex ("#w_wrist.md2");		//wrist rocket
	gi.modelindex ("#w_mtube.md2");		//missile tube
	gi.modelindex ("#w_dis.md2");		//disruptor
	gi.modelindex ("#w_nstg.md2");		//nightstinger
	gi.modelindex ("#w_beam.md2");		//beamtube
	gi.modelindex ("#w_thrm.md2");		//thermal
	gi.modelindex ("#w_bfg.md2");		//hands(nothing)
	gi.modelindex ("#w_sabr.md2");		//light saber

	gi.imageindex ("weapons/wm_pist1");
	gi.imageindex ("weapons/wm_pist2");
	gi.imageindex ("weapons/wm_trpr1");
	gi.imageindex ("weapons/wm_trpr2");
	gi.imageindex ("weapons/wm_rapid1");
	gi.imageindex ("weapons/wm_rapid2");
	gi.imageindex ("weapons/wm_arrw1");
	gi.imageindex ("weapons/wm_arrw2");
	gi.imageindex ("weapons/wm_tdlauncher1");
	gi.imageindex ("weapons/wm_tdlauncher2");
	gi.imageindex ("weapons/wm_wrstrkt1");
	gi.imageindex ("weapons/wm_wrstrkt2");
	gi.imageindex ("weapons/wm_mtube1");
	gi.imageindex ("weapons/wm_mtube2");
	gi.imageindex ("weapons/wm_dis1");
	gi.imageindex ("weapons/wm_dis2");
	gi.imageindex ("weapons/wm_nstg1");
	gi.imageindex ("weapons/wm_nstg2");
	gi.imageindex ("weapons/wm_beam1");
	gi.imageindex ("weapons/wm_beam2");
	gi.imageindex ("weapons/wm_thrm1");
	gi.imageindex ("weapons/wm_thrm2");
	gi.imageindex ("weapons/wm_sabr1");
	gi.imageindex ("weapons/wm_sabr2");

	gi.imageindex ("weapons/menu1");
	gi.imageindex ("weapons/menu2");
	gi.imageindex ("weapons/menu3");
	gi.imageindex ("weapons/menu4");

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
	gi.soundindex ("noammo.wav");

	gi.soundindex ("The Force.wav");

	gi.soundindex ("infantry/inflies1.wav");

//	gi.soundindex ("force.wav");

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

