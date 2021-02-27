#include "g_local.h"

typedef struct
{
	char	*name;
	void	(*spawn)(edict_t *ent);
} spawn_t;
char *LoadEntities(char *mapname, char *entities);//K03

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
void SP_monster_tank_commander(edict_t *ent);
void SP_monster_floater (edict_t *self);
void SP_monster_hover (edict_t *self);
void SP_monster_mutant (edict_t *self);
void SP_monster_supertank (edict_t *self);
void SP_monster_boss2 (edict_t *self);
void SP_monster_jorg (edict_t *self);
void SP_monster_boss3_stand (edict_t *self);
void SP_misc_insane (edict_t *self);

void SP_monster_commander_body (edict_t *self);

void SP_turret_breach (edict_t *self);
void SP_turret_base (edict_t *self);
void SP_turret_driver (edict_t *self);

// RAFAEL 14-APR-98
void SP_monster_soldier_hypergun (edict_t *self);
void SP_monster_soldier_lasergun (edict_t *self);
void SP_monster_soldier_ripper (edict_t *self);
void SP_monster_fixbot (edict_t *self);
void SP_monster_gekk (edict_t *self);
void SP_monster_chick_heat (edict_t *self);
void SP_monster_gladb (edict_t *self);
void SP_monster_boss5 (edict_t *self);
void SP_rotating_light (edict_t *self);
void SP_object_repair (edict_t *self);
void SP_misc_crashviper (edict_t *ent);
void SP_misc_viper_missile (edict_t *self);
void SP_misc_amb4 (edict_t *ent);
void SP_target_mal_laser (edict_t *ent);
void SP_misc_transport (edict_t *ent);
// END 14-APR-98
//GHz START
void SP_info_player_invasion (edict_t *self);
void SP_info_monster_invasion (edict_t *self);
void SP_navi_monster_invasion (edict_t *self);
void SP_inv_defenderspawn (edict_t *self);
//GHz END

void SP_misc_nuke (edict_t *ent);

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

	// RAFAEL
	{"func_object_repair", SP_object_repair},
	{"rotating_light", SP_rotating_light},

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
#if 0 // remove monster code
	{"target_actor", SP_target_actor},
#endif
	{"target_lightramp", SP_target_lightramp},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

	// RAFAEL 15-APR-98
	{"target_mal_laser", SP_target_mal_laser},

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
#if 0 // remove monster code
	{"misc_actor", SP_misc_actor},
#endif
	{"misc_gib_arm", SP_misc_gib_arm},
	{"misc_gib_leg", SP_misc_gib_leg},
	{"misc_gib_head", SP_misc_gib_head},
#if 0 // remove monster code
	{"misc_insane", SP_misc_insane},
#endif
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

	// RAFAEL
//	{"misc_crashviper", SP_misc_crashviper},
//	{"misc_viper_missile", SP_misc_viper_missile},
	{"misc_amb4", SP_misc_amb4},
	// RAFAEL 17-APR-98
	{"misc_transport", SP_misc_transport},
	// END 17-APR-98
	// RAFAEL 12-MAY-98
	{"misc_nuke", SP_misc_nuke},

	{"monster_berserk", SP_monster_berserk},
	{"monster_gladiator", SP_monster_gladiator},
	{"monster_gunner", SP_monster_gunner},
	{"monster_soldier", SP_monster_soldier},
	{"monster_tank", SP_monster_tank},
	{"monster_tank_commander", SP_monster_tank_commander},
	{"monster_medic", SP_monster_medic},
	{"monster_chick", SP_monster_chick},
	{"monster_parasite", SP_monster_parasite},
	{"monster_brain", SP_monster_brain},
	{"monster_mutant", SP_monster_mutant},
	{"monster_infantry", SP_monster_infantry},
	// {"monster_soldier_light", SP_monster_soldier_light},

#if 0 // remove monster code
	{"monster_flyer", SP_monster_flyer},
	{"monster_floater", SP_monster_floater},
	{"monster_hover", SP_monster_hover},
	{"monster_supertank", SP_monster_supertank},
	{"monster_boss2", SP_monster_boss2},
	{"monster_boss3_stand", SP_monster_boss3_stand},
	{"monster_jorg", SP_monster_jorg},
	{"monster_flipper", SP_monster_flipper},
	{"monster_commander_body", SP_monster_commander_body},

	{"turret_breach", SP_turret_breach},
	{"turret_base", SP_turret_base},
	{"turret_driver", SP_turret_driver},
#endif
//GHz START
	{"info_player_invasion", SP_info_player_invasion},
	{"info_monster_invasion", SP_info_monster_invasion},
	{"navi_monster_invasion", SP_navi_monster_invasion},
	{"inv_defenderspawn", SP_inv_defenderspawn},
//GHz END
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

	// check item spawn functions
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++)
	{
		if (!item->classname)
			continue;
		//gi.dprintf("processing item: %s\n", item->classname);
		if (!strcmp(item->classname, ent->classname))
		{	// found it
			SpawnItem (ent, item);
			return;
		}
	}

	// check normal spawn functions
	for (s=spawns ; s->name ; s++)
	{
		//if (ent->classname)
		//	gi.dprintf("processing %s\n", ent->classname);
		if (!strcmp(s->name, ent->classname))
		{	// found it
			s->spawn (ent);
			return;
		}
	}
	//gi.dprintf ("%s doesn't have a spawn function\n", ent->classname);
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

	newb = V_Malloc (l, TAG_LEVEL);

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
		if (!Q_stricmp(f->name, key))
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
//	gi.dprintf ("%s is not a field %s\n", key,ent->classname);
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
================
G_FindTrainTeam

Chain together all entities with a matching team field.

All but the last will have the teamchain field set to the next one
================
*/
void G_FindTrainTeam()
{
	edict_t	*teamlist[MAX_EDICTS + 1];	
	edict_t	*e,*t,*p;

	qboolean	findteam;
	char	*currtarget,*currtargetname;
	char	*targethist[MAX_EDICTS];
	int		lc,i,j,k;
	int		loopindex;

	e = &g_edicts[(int)maxclients->value+1];
	for ( i=maxclients->value+1 ; i<globals.num_edicts ; i++, e++)
	{
		if(e->inuse && e->classname)
		{
			if(!Q_stricmp(e->classname,"path_corner") && e->targetname && e->target)
			{
//		orgtarget = e->target;		//terminal
//		orgtargetname = e->targetname;
				currtarget = e->target;			//target
				currtargetname = e->targetname;	//targetname
				lc = 0;

				memset(&teamlist,0,sizeof(teamlist));
				memset(&targethist,0,sizeof(targethist));
				findteam = false;

				loopindex = 0;
				targethist[0] = e->targetname;

				while(lc < MAX_EDICTS)
				{
					t = &g_edicts[(int)maxclients->value+1];
					for ( j=maxclients->value+1 ; j<globals.num_edicts ; j++, t++)
					{
						if(t->inuse && t->classname)
						{
							if(!Q_stricmp(t->classname,"func_train") 
								&& !Q_stricmp(t->target,currtargetname)
								&& t->trainteam == NULL)
							{
								for(k = 0;k < lc; k++)
								{
									if(teamlist[k] == t) break;
								}
								if(k == lc )
								{
									teamlist[lc] = t;
									lc++;
								}
							}
						}
					}
					p = G_PickTarget(currtarget);
					if(!p) break;
					currtarget = p->target;
					currtargetname = p->targetname;
					if(!p->target) break;
					for(k = 0;k < loopindex;k++)
					{
						if(!Q_stricmp(targethist[k],currtargetname)) break;
					}
					if(k < loopindex)
					{
						findteam = true;
						break;
					}
					targethist[loopindex] = currtargetname;
					loopindex++;
					/*if(!Q_stricmp(currtarget,orgtargetname))
					{
						findteam = true;
						break;
					}*/
				}
				if(findteam && lc > 0)
				{
					gi.dprintf("%i train chaining founded.\n",lc);
					for(k = 0;k < lc; k++)
					{
						if(teamlist[k + 1] == NULL)
						{
							teamlist[k]->trainteam = teamlist[0];
							break;
						}
						teamlist[k]->trainteam = teamlist[k + 1];
					}
				}
			}
		}
	}
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void InitScanEntity (void);
void SpawnWorldAmmo (void);
void InitSunEntity(void);
qboolean vrx_CheckForFlag (void);
void CreateGrid(qboolean force);
void DroneList_Clear();

void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	int			laser = 0;
	int			saved;//4.5 don't lose level.r_monsters value!

	SaveClientData ();

	gi.FreeTags (TAG_LEVEL);

	maplist.sounds[1] = maplist.sounds[2] = maplist.sounds[3] = 0;

	saved = level.r_monsters;//4.5
	memset (&level, 0, sizeof(level));
	memset (g_edicts, 0, game.maxentities * sizeof (g_edicts[0]));

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);
	strncpy (game.spawnpoint, spawnpoint, sizeof(game.spawnpoint)-1);

	// set client fields on player ents
	for (i=0 ; i<game.maxclients ; i++)
		g_edicts[i+1].client = game.clients + i;

	ent = NULL;
	inhibit = 0;

	DroneList_Clear();

	//K03 Begin
	INV_Init();
	entities = LoadEntities(mapname, entities);
	//K03 End

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
		
		// remove things (except the world) from different skill levels or deathmatch
		if (ent != g_edicts)
		{
			if (deathmatch->value)
			{
				if ( (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH) ||
					((!invasion->value) && ent->spawnflags & SPAWNFLAG_INVASION_ONLY) )
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

			ent->spawnflags &= ~(SPAWNFLAG_NOT_EASY|SPAWNFLAG_NOT_MEDIUM|SPAWNFLAG_NOT_HARD|SPAWNFLAG_INVASION_ONLY|SPAWNFLAG_NOT_DEATHMATCH);
		}

		ED_CallSpawn (ent);
	}

//	gi.dprintf ("%i entities inhibited\n", inhibit);

	AI_NewMap();//JABot

	RunLuaMapSettings(mapname);

	level.r_monsters = Lua_GetVariable(va("%s_monsters", mapname), saved);
	level.pathfinding = Lua_GetVariable(va("%s_UsePathfinding", level.mapname), 0) || Lua_GetVariable(va("UsePathfinding", level.mapname), 0);

	G_FindTeams ();

	PlayerTrail_Init ();
	G_FindTrainTeam();
//GHz START
	InitScanEntity();
	InitSunEntity();

	if (!coop->value)
		InitMonsterEntity(false);

	PrintNumEntities(false);
	SpawnWorldAmmo();

	if (level.pathfinding)
		CreateGrid(false);

//GHz END

	if (invasion->value)
	{
		gi.soundindex("5_0.wav");
		gi.soundindex("invasion/fight_invasion.wav");
		if (invasion->value > 1)
			gi.soundindex("invasion/hard_victory.wav");
	}

	gi.soundindex("invasion/30sec.wav");
	gi.soundindex("invasion/20sec.wav");
	gi.soundindex("world/10_0.wav");
	gi.soundindex("misc/talk1.wav");
	gi.soundindex("misc/tele_up.wav");
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
/*
GHz's NOTES:
All characters have a width and height of 8. Numbers have a width of 15
and a height of 24. The virtual Y starts at the top of the screen and
goes down, while the virtual X position of the screen starts at the left
side of the screen. Numbers go from right (at 0) to left, while strings
go from left to right. Confusing eh?
*/
char *single_statusbar = 
"yb	-48 "

// health
"xl	200 "
"num 3 1 "
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
"	yv	125 "
"   xv 120 "
"	pic 7 "
"	xv	145 "
"	yb	133 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
"	xl	0 "
"   yb -96 "
"	num	2	10 "
"	xv	296 "
"	pic	9 "
"   yb -50 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "
;

char *dm_statusbar =
"yb	-72 "
// health
"xl	24 "
"num 4 1 "

// ammo
"if 2 "
"	xl 24 "
"	yb	-48 "//K03 was 100
"	anum "
"	xl 0 "//K03 was 150
"   yb -72 "
"	pic 2 "
"endif "

// armor
"yb -24 "
"xl 24 "//K03 was 200
"rnum "
"endif "

// selected item
"ifeq 29 0 "
"if 6 "
//K03 Begin
"   xr  -72 "
"   num 3 27 "
//K03 End
"	xr -24 "
"	pic 6 "
"endif "

"endif "

"yb	-50 "

// picked up item
"if 7 "
//"   yv 125 "
"	yv -42 "
"	xv 120 "
"	pic 7 "
"	xv 145 "
"	stat_string 8 "
"	yb	-50 "
"endif "

// timer
"if 9 "
//K03 Begin
//"	xv	246 "
" yb -96 "
" xl 25 "
"	num	3 10 "//was " num 2 10 "
//K03 End
"	xl	0 "
"	pic	9 "
"   yb -50 "
"endif "

//  help / weapon icon 
"if 11 "
"	xv	148 "
"	pic	11 "
"endif "
//K03 Begin

//  frags
"xr -34 "
"yb -116 "
"string \"Exp\" "

"xr	-81 "
"yb -108 "
"num 5 14 "
//K03 End
// spectator
"if 29 "
  "xv 0 "
  "yb -58 "
  "string2 \"SPECTATOR MODE\" "
"endif "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
 // "string \"Follow\" "
  "xv 0 "
  "stat_string 16 "
"endif "

// team number
"if 17 "
"xr	-20 "
"yt 107 "
"pic 17 "
"endif "

// supply station icon
"if 18 "
"xl 0 "
"yt 46 "
"pic 18 "
// supply station time
"xl	30 "
"yt 46 "
"num 3 19 "
"endif "
/*
//K03 Begin rank order system
//  rank
"xr -34 "
"yt 37 "
"string \"Rank\" "

"xr	-50 "
"yt 46 "
"num 3 24 "
//Rank stuff done
*/
// az start
// invasion time left
"if 21 "
"   xv 136 "
"   yv  252 "
"   string2 \"Time Left\" "
"   xv 136 "
"   yv 260 "
"   num 4 21 "
"endif "
// vote notifier
"if 22 "
"   xv 0 "
"   yb  -60 "
"   stat_string 22 "
"endif "
// az end

//GHz START
// show damage done to target
"if 24 "
"   xv 136 "
"   yv  150 "
"   string2 \"DMG\" "
"   xv 130 "
"   yv 159  "
"   num 5 24 "
"endif "
//GHz END
//Show the Streak
"xr -42 "
"yb -86 "
"string \"Kills\" "
//  total players
"xr -50 "
"yb	-78 "
"num 3 25 "
//End streak stuff
//power cubes left
"xr -42 "
"yb -56 "
"string \"Mana\" "

"xr -68 "
"yb -48 "
"num 4 28 "

//print out our current level
"xr -25 "
"yt 158 "
"string \"Level\" "
/*
"xr -50 "
"yt 167 "//150
"num 3 26 "
*/
//K03 End
//GHz START

// health-id
"if 30 "
"   xv 0 "
"   yb  -157 "
"   string2 \"Life\" "

"   xv 0 "
"   yb -148  "
"   num 4 30 "
"endif "

// armor-id
"if 26 "
"   xv 0 "
"   yb  -124 "
"   string2 \"Armor\" "
"endif "

"if 26 "
"   xv 0 "
"   yb -115 "
"   num 4 26 "
"endif "

// ammo-id
"if 31 "
"   xv 0 "
"   yb  -91 "
"   string2 \"Ammo\" "
"endif "

"if 31 "
"   xv 0 "
"   yb -82 "
"   num 4 31 "
"endif "

"if 24 "
"   xv 136 "
"   yv  150 "
"   string2 \"DMG\" "
"   xv 130 "
"   yv 159  "
"   num 5 24 "
"endif "

// 3.5 show ability charge percent
"if 20 "
"   xr -50 "
"   yt  133 "
"   string2 \"Charge\" "
"   xr -50 "
"   yt 142 "
"   num 3 20 "
"endif "
//GHz END
;

edict_t *SpawnDroneEnt (edict_t *drone, edict_t *ent, int drone_type, qboolean worldspawn);

void SP_monster_berserk(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 9, true);
}

void SP_monster_gladiator(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 8, true);
}

void SP_monster_gunner(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 1, true);
}

void SP_monster_soldier(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 10, true);
}

void SP_monster_tank(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 6, true);
}

void SP_monster_tank_commander(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 30, true);
}

void SP_monster_infantry(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 11, true);
}

void SP_monster_medic(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 5, true);
}

void SP_monster_mutant(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 7, true);
}

void SP_monster_chick(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 3, true);
}

void SP_monster_parasite(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 2, true);
}

void SP_monster_brain(edict_t *ent) 
{
	if (coop->value)
		SpawnDroneEnt(ent, g_edicts, 4, true);
}


/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
// ***** GHz PRECACHE ALL MODELS AND SOUNDS HERE *****
void SP_worldspawn (edict_t *ent)
{
	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
	ent->inuse = true;			// since the world doesn't use G_Spawn()
	ent->s.modelindex = 1;		// world model is always index 1

	//---------------

	// reserve some spots for dead player bodies
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

	gi.imageindex("a_blaster_hud");
	gi.imageindex("a_shells_hud");
	gi.imageindex("a_bullets_hud");
	gi.imageindex("a_grenades_hud");
	gi.imageindex("a_rockets_hud");
	gi.imageindex("a_cells_hud");
	gi.imageindex("a_slugs_hud");

	gi.soundindex ("world/klaxon2.wav");
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

/*	if (coop->value || deathmatch->value)
	{*/
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

		gi.modelindex ("#w_phalanx.md2");
		gi.modelindex ("#w_ripper.md2"); //downloading file cuando uno inicia el juego
//	}


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
	skullindex = gi.modelindex ("models/objects/gibs/skull/tris.md2");
	headindex = gi.modelindex ("models/objects/gibs/head2/tris.md2");

	gi.modelindex("models/proj/beam/tris.md2"); // 3.7 heatbeam model
	//gi.modelindex("models/proj/lightning/tris.md2"); //3.9 lightning model used by hellspawn/holyshock

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

//----------------------------------------------


	//pre searched items
	Fdi_GRAPPLE			= FindItem ("Grapple");
	Fdi_BLASTER			= FindItem ("Blaster");
	Fdi_SHOTGUN			= FindItem ("Shotgun");
	Fdi_SUPERSHOTGUN	= FindItem ("Super Shotgun");
	Fdi_MACHINEGUN		= FindItem ("Machinegun");
	Fdi_CHAINGUN		= FindItem ("Chaingun");
	Fdi_GRENADES		= FindItem ("Grenades");
	Fdi_GRENADELAUNCHER	= FindItem ("Grenade Launcher");
	Fdi_ROCKETLAUNCHER	= FindItem ("Rocket Launcher");
	Fdi_HYPERBLASTER	= FindItem ("HyperBlaster");
	Fdi_RAILGUN			= FindItem ("Railgun");
	Fdi_BFG				= FindItem ("BFG10K");
	Fdi_20MM			= FindItem ("20mm Cannon");

	Fdi_SHELLS			= FindItem ("Shells");
	Fdi_BULLETS			= FindItem ("Bullets");
	Fdi_CELLS			= FindItem ("Cells");
	Fdi_ROCKETS			= FindItem ("Rockets");
	Fdi_SLUGS			= FindItem ("Slugs");

	//K03 Begin
	Fdi_POWERCUBE		= FindItem("Power Cube");
	Fdi_TBALL			= FindItem("tballs");
	//K03 End

	if (invasion->value || pvm->value || ffa->value)
	{
		gi.soundindex ("berserk/berpain2.wav");
		gi.soundindex ("berserk/berdeth2.wav");
		gi.soundindex ("berserk/beridle1.wav");
		gi.soundindex ("berserk/attack.wav");
		gi.soundindex ("berserk/bersrch1.wav");
		gi.soundindex ("berserk/sight.wav");

		gi.modelindex("models/monsters/berserk/tris.md2");

		gi.soundindex ("chick/chkatck1.wav");	
		gi.soundindex ("chick/chkatck2.wav");	
		gi.soundindex ("chick/chkatck3.wav");	
		gi.soundindex ("chick/chkatck4.wav");	
		gi.soundindex ("chick/chkatck5.wav");	
		gi.soundindex ("chick/chkdeth1.wav");	
		gi.soundindex ("chick/chkdeth2.wav");	
		gi.soundindex ("chick/chkfall1.wav");	
		gi.soundindex ("chick/chkidle1.wav");	
		gi.soundindex ("chick/chkidle2.wav");	
		gi.soundindex ("chick/chkpain1.wav");	
		gi.soundindex ("chick/chkpain2.wav");	
		gi.soundindex ("chick/chkpain3.wav");	
		gi.soundindex ("chick/chksght1.wav");	
		gi.soundindex ("chick/chksrch1.wav");	
		gi.modelindex ("models/monsters/bitch/tris.md2");


		gi.soundindex ("brain/brnatck1.wav");

		gi.soundindex ("brain/brnatck2.wav");

		gi.soundindex ("brain/brnatck3.wav");

		gi.soundindex ("brain/brndeth1.wav");

		gi.soundindex ("brain/brnidle1.wav");

		gi.soundindex ("brain/brnidle2.wav");

		gi.soundindex ("brain/brnlens1.wav");

		gi.soundindex ("brain/brnpain1.wav");
		gi.soundindex ("brain/brnpain2.wav");
		gi.soundindex ("brain/brnsght1.wav");
		gi.soundindex ("brain/brnsrch1.wav");
		gi.soundindex ("brain/melee1.wav");
		gi.soundindex ("brain/melee2.wav");
		gi.soundindex ("brain/melee3.wav");
		gi.soundindex ("player/land1.wav");

		gi.modelindex ("models/monsters/brain/tris.md2");
		

		gi.soundindex ("gladiator/glddeth2.wav");	
		gi.soundindex ("gladiator/railgun.wav");
		gi.soundindex ("gladiator/melee1.wav");
		gi.soundindex ("gladiator/melee2.wav");
		gi.soundindex ("gladiator/melee3.wav");
		gi.soundindex ("gladiator/gldidle1.wav");
		gi.soundindex ("gladiator/gldsrch1.wav");
		gi.soundindex ("gladiator/sight.wav");

		gi.modelindex ("models/monsters/gladiatr/tris.md2");

		gi.soundindex ("gunner/death1.wav");	
		gi.soundindex ("gunner/gunpain2.wav");	
		gi.soundindex ("gunner/gunpain1.wav");	
		gi.soundindex ("gunner/gunidle1.wav");	
		gi.soundindex ("gunner/gunatck1.wav");	
		gi.soundindex ("gunner/gunsrch1.wav");	
		gi.soundindex ("gunner/sight1.wav");
		gi.soundindex ("player/land1.wav");

		gi.soundindex ("gunner/gunatck2.wav");
		gi.soundindex ("gunner/gunatck3.wav");


		gi.soundindex ("medic/idle.wav");
		gi.soundindex ("medic/medpain1.wav");
		gi.soundindex ("medic/medpain2.wav");
		gi.soundindex ("medic/meddeth1.wav");
		gi.soundindex ("medic/medsght1.wav");
		gi.soundindex ("medic/medsrch1.wav");
		gi.soundindex ("medic/medatck2.wav");
		gi.soundindex ("medic/medatck3.wav");
		gi.soundindex ("medic/medatck4.wav");
		gi.soundindex ("medic/medatck5.wav");

		gi.soundindex ("medic/medatck1.wav");

		gi.soundindex ("mutant/mutatck1.wav");
		gi.soundindex ("mutant/mutatck2.wav");
		gi.soundindex ("mutant/mutatck3.wav");
		gi.soundindex ("mutant/mutdeth1.wav");
		gi.soundindex ("mutant/mutidle1.wav");
		gi.soundindex ("mutant/mutsght1.wav");
		gi.soundindex ("mutant/step1.wav");
		gi.soundindex ("mutant/step2.wav");
		gi.soundindex ("mutant/step3.wav");
		gi.soundindex ("mutant/thud1.wav");

		gi.modelindex ("models/monsters/mutant/tris.md2");

		gi.soundindex ("parasite/parpain1.wav");	
		gi.soundindex ("parasite/parpain2.wav");	
		gi.soundindex ("parasite/pardeth1.wav");	
		gi.soundindex("parasite/paratck1.wav");
		gi.soundindex("parasite/paratck2.wav");
		gi.soundindex("parasite/paratck3.wav");
		gi.soundindex("parasite/paratck4.wav");
		gi.soundindex("parasite/parsght1.wav");
		gi.soundindex("parasite/paridle1.wav");
		gi.soundindex("parasite/paridle2.wav");
		gi.soundindex("parasite/parsrch1.wav");

		gi.modelindex ("models/monsters/parasite/tris.md2");

		gi.modelindex ("models/monsters/tank/tris.md2");

		gi.soundindex ("tank/tnkpain2.wav");
		gi.soundindex ("tank/tnkdeth2.wav");
		gi.soundindex ("tank/tnkidle1.wav");
		gi.soundindex ("tank/death.wav");
		gi.soundindex ("tank/step.wav");
		gi.soundindex ("tank/tnkatck4.wav");
		gi.soundindex ("tank/tnkatck5.wav");
		gi.soundindex ("tank/sight1.wav");

		gi.soundindex ("tank/tnkatck1.wav");
		gi.soundindex ("tank/tnkatk2a.wav");
		gi.soundindex ("tank/tnkatk2b.wav");
		gi.soundindex ("tank/tnkatk2c.wav");
		gi.soundindex ("tank/tnkatk2d.wav");
		gi.soundindex ("tank/tnkatk2e.wav");
		gi.soundindex ("tank/tnkatck3.wav");
	}	
}


