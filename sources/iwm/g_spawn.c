/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"
//#include "windows.h"

#ifdef _WIN32
#include "winbase.h"
#endif
#include <time.h>

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

void SP_func_vehicle (edict_t *self);
//void SP_item_botroam (edict_t *self);	//JABot

spawn_t	spawns[] = {
	{"item_health", SP_item_health},
	{"item_health_small", SP_item_health_small},
	{"item_health_large", SP_item_health_large},
	{"item_health_mega", SP_item_health_mega},

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
	{"func_vehicle", SP_func_vehicle},

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
//ZOID
	{"trigger_teleport", SP_trigger_teleport},
	{"info_teleport_destination", SP_info_teleport_destination},
//ZOID

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
//	{"item_botroam", SP_item_botroam},	//JABot

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
	for(;;)
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
void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void drop_make_touchable (edict_t *ent);

// Paril, finds a random place on the map to put something.
// FIXME: On small maps, this does not work. There's got to be a better way...
// MEFIXED: Muhahaha. Came up with a fix!
qboolean FindSmallPoint (edict_t *ent)
{
	vec3_t loc = {0,0,0};
	vec3_t floor;
	int i;
	int j = 0;
	int k = 0;
	trace_t tr;
	vec3_t spawn_origin, spawn_angles;

	do {
		j++;
		for (i = 0; i < 3; i++)
			loc[i] = rand() % (1000 + 1) - 500;
		
		if (gi.pointcontents(loc) == 0)
		{
			VectorCopy(loc, floor);
			floor[2] = -500;
			tr = gi.trace (loc, vec3_origin, vec3_origin, floor, NULL, MASK_SOLID);
			k++;
			if (tr.contents & MASK_WATER)
				continue; 
			VectorCopy (tr.endpos, loc);
			loc[0] += ent->maxs[0] - ent->mins[0]; // make sure the entity can fit!
			loc[1] += ent->maxs[1] - ent->mins[1]; // make sure the entity can fit!
			loc[2] += ent->maxs[2] - ent->mins[2]; // make sure the entity can fit!
		}
	} while (gi.pointcontents(loc) > 0 && j < 1000 && k < 500);
	
	// STILL not good?
	if (j >= 1000 || k >= 500)
	{
		if (!ent->client)
			gi.dprintf ("%s couldn't find a spawnpoint: FindSmallPoint\n", ent->classname);
		else
			gi.dprintf ("%s couldn't find a spawnpoint: FindSmallPoint\n", ent->client->pers.netname);

		SelectSpawnPoint (ent, spawn_origin, spawn_angles);
		VectorCopy (spawn_origin, ent->s.origin);
		VectorCopy (spawn_angles, ent->s.angles);
		return false;
	}

	loc[2] += 82;
	VectorCopy(loc,ent->s.origin);
	VectorCopy(loc,ent->s.old_origin);
	return true;
}

qboolean findspawnpoint (edict_t *ent)
{
	vec3_t loc = {0,0,0};
	vec3_t floor;
	int i;
	int j = 0;
	int k = 0;
	trace_t tr;
	do {
		j++;
		for (i = 0; i < 3; i++)
			loc[i] = rand() % (8192 + 1) - 4096;
		
		if (gi.pointcontents(loc) == 0)
		{
			VectorCopy(loc, floor);
			floor[2] = -4096;
			tr = gi.trace (loc, vec3_origin, vec3_origin, floor, NULL, MASK_SOLID);
			k++;
			if (tr.contents & MASK_WATER)
				continue; 
			VectorCopy (tr.endpos, loc);
			loc[0] += ent->maxs[0] - ent->mins[0]; // make sure the entity can fit!
			loc[1] += ent->maxs[1] - ent->mins[1]; // make sure the entity can fit!
			loc[2] += ent->maxs[2] - ent->mins[2]; // make sure the entity can fit!
		}
	} while (gi.pointcontents(loc) > 0 && j < 1000 && k < 500);
	
	// Paril: Fixed where small maps are not working with this code!
//	gi.dprintf ("%f\n", gi.pointcontents(tr.endpos));
	if (!gi.pointcontents(tr.endpos) == 0)
	{
		// If the object went inside a solid here, we find an even smaller point!
		// FIXME: Is this NEW func needed? Can I just use a goto and a variable for size?
		FindSmallPoint (ent);
		return false;
	}

	loc[2] += 82;
	VectorCopy(loc,ent->s.origin);
	VectorCopy(loc,ent->s.old_origin);
	return true;
}

void FollowOwner (edict_t *s)
{
	vec3_t add = {0, 0, 20};

	if (!s->owner->inuse)
	{
		G_FreeEdict (s);
		return;
	}

	VectorCopy (s->owner->s.origin, s->s.origin);
	VectorAdd (s->s.origin, add, s->s.origin);
	gi.linkentity (s);

	s->s.angles[0] += 10;
	s->s.angles[1] -= 10;
	//s->s.angles[2] += 15;
	s->nextthink = level.time + .1;
}

void HoldSecondPart (edict_t *owner)
{	
	vec3_t add = {0, 0, 20};
	edict_t *p = owner->tempent;
	p = G_Spawn();
	p->movetype = MOVETYPE_NONE;
	p->owner = owner;
	VectorCopy (p->owner->s.origin, p->s.origin);
	VectorAdd (p->s.origin, add, p->s.origin);
	if (Q_stricmp(owner->item->classname, "item_cloaking") == 1)
		p->s.modelindex = ModelIndex ("models/items/regen/tris_outer.md2");
	else
		p->s.modelindex = ModelIndex ("models/items/invis/outer.md2");
	p->think = FollowOwner;
	p->nextthink = level.time + .1;
	p->nextthink2 = level.time + 30;
	p->think2 = G_FreeEdict;
	p->s.renderfx |= RF_IR_VISIBLE;

	owner->tempent = p;

	gi.linkentity (p);
}

void AddSkin (edict_t *self)
{
	trace_t tr;
	vec3_t end, star;

	self->s.effects |= EF_TAGTRAIL;

	VectorCopy(self->s.origin, star);

	VectorMA(star, 0.5, tv(0, 0, -90), end);
	tr = gi.trace(star, NULL, NULL, end, self, MASK_SOLID);

	if (tr.fraction < 1.0)
	{
		//gi.dprintf ("Hit a plane: vel2 %f\n", self->velocity[2]);
		self->velocity[2] = 450;
	}

	if ((self->s.skinnum + 1) == 2)
		self->s.skinnum = 0;
	else
		self->s.skinnum = 1;

	self->nextthink2 = level.time + .1;
}

edict_t *SpawnTheActualItem (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;

	dropped = G_Spawn();

	dropped->inuse = true;
	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	//gi.dprintf ("%s\n", dropped->item->world_model);
	if (dropped->item->world_model)
		//gi.setmodel (dropped, dropped->item->world_model);
		dropped->s.modelindex = ModelIndex (dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;
	dropped->s.renderfx |= RF_IR_VISIBLE;
	dropped->tempfloat = 3;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;
	if (Q_stricmp(item->classname, "regeneration") == 0 || Q_stricmp(item->classname, "item_cloaking") == 0)
	{
		dropped->think2 = HoldSecondPart;
		dropped->nextthink2 = level.time + .1;
	}
	if (Q_stricmp(item->classname, "item_star") == 0)
	{
		dropped->velocity[0] += 360;
		dropped->velocity[1] += 360;
		dropped->velocity[2] += 360;
		dropped->think2 = AddSkin;
		dropped->nextthink2 = level.time + .5;
		dropped->movetype = MOVETYPE_BOUNCESTAY;
		dropped->s.origin[2] += 48;
		dropped->s.effects |= EF_TAGTRAIL;
	}

	gi.linkentity (dropped);

	findspawnpoint(ent);

	return dropped;
}

void SpawnTheItems (edict_t *s)
{
	gitem_t *one = FindItem("Ir Goggles");
	gitem_t *two = FindItem("Jump Boots");
	gitem_t *three = FindItem("Regeneration");
	gitem_t *four = FindItem("Donkey Kong Hammer");
	gitem_t *five = FindItem("Transparency Device");
	gitem_t *six = FindItem("Magic Carpet");
	gitem_t *seven = FindItem("Deflector Shield");
	gitem_t *eight = FindItem("Starman");
	edict_t *g;
	edict_t *g2;
	edict_t *g3;
	edict_t *g4;
	edict_t *g5;
	edict_t *g6;
	edict_t *g7;
	edict_t *g8;

	// Fugget about it, get outta here!
	if (item_ban->string && Q_stricmp(item_ban->string, "all") == 0)
	{
		s->nextthink = level.time + 29;
		return;
	}

	if (random() > 0.4 && !CheckBan(item_ban, ITEMBAN_IR_GOGGLES))
	{
		g = SpawnTheActualItem (s, one);
		findspawnpoint(g);
	}
	if (random() > 0.4 && !CheckBan(item_ban, ITEMBAN_JUMP_BOOTS))
	{
		g2 = SpawnTheActualItem (s, two);
		findspawnpoint(g2);
	}
	if (random() > 0.4 && !CheckBan(item_ban, ITEMBAN_REGENERATION))
	{
		g3 = SpawnTheActualItem (s, three);
		findspawnpoint(g3);
	}
	if (random() > 0.4 && !CheckBan(item_ban, ITEMBAN_DONKEY_KONG_HAMMER))
	{
		g4 = SpawnTheActualItem (s, four);
		findspawnpoint(g4);
	}
	if (random() < 0.8 && !CheckBan(item_ban, ITEMBAN_TRANSPARENCY_DEVICE))
	{
		g5 = SpawnTheActualItem (s, five);
		findspawnpoint(g5);
		//g5->s.effects &= ~EF_SPHERETRANS;
		g5->s.renderfx |= RF_TRANSLUCENT;
		//gi.dprintf ("Spawned trans\n");
	}
	if (random() > 0.4 && !CheckBan(item_ban, ITEMBAN_MAGIC_CARPET))
	{
		g6 = SpawnTheActualItem (s, six);
		findspawnpoint(g6);
		//gi.dprintf ("Test\n");
	}
	if (random() > 0.4 && !CheckBan(item_ban, ITEMBAN_DEFLECTOR_SHIELD))
	{
		g7 = SpawnTheActualItem (s, seven);
		findspawnpoint(g7);
		//gi.dprintf ("Test\n");
	}
	if (random() < 0.18 && !CheckBan(item_ban, ITEMBAN_STARMAN))
	{
		g8 = SpawnTheActualItem (s, eight);
		findspawnpoint(g8);
		//gi.dprintf ("Test\n");
	}

	s->nextthink = level.time + 29;
}

void SpawnTheSpawner (void)
{
	edict_t *spawner;

	if (iwm_editor->value)
		return;

	spawner = G_Spawn();
	spawner->think = SpawnTheItems;
	spawner->nextthink = level.time + 3;
	spawner->classname = "spawnit";

	gi.linkentity(spawner);
}

int total_monsters;

void SpawnAMonster (edict_t *s)
{
	edict_t *mon;
	int mtype;

	if (total_monsters == 15)
	{
		s->nextthink = level.time + 1;
		return;
	}

	mtype = rand()%8;

	mon = G_Spawn();
	mon->is_spawned = 1;

	if (mtype == 0)
	{
		SP_monster_soldier(mon);
		mon->name = "a Soldier";
	}
	else if (mtype == 1)
	{
		SP_monster_soldier_ss(mon);
		mon->name = "a Machinegun Soldier";
	}
	else if (mtype == 2)
	{
		mon->name = "a Gunner";
		SP_monster_gunner(mon);
	}
	else if (mtype == 3)
	{
		mon->name = "a Mutant";
		SP_monster_mutant(mon);
	}
	else if (mtype == 4)
	{
		mon->name = "a Gladiator";
		SP_monster_gladiator(mon);
	}
	else if (mtype == 5)
	{
		mon->name = "a Medic";
		SP_monster_medic(mon);
	}
	else if (mtype == 6)
	{
		if (random() < 0.7)
			mon->classname = "monster_tank_commander";
		else
			mon->classname = "monster_tank";

		if (Q_stricmp(mon->classname, "monster_tank_commander") == 0)
			mon->name = "a Tank Commander";
		else
			mon->name = "a Tank";

		SP_monster_tank(mon);

		if (Q_stricmp(mon->name, "a Tank Commander") == 0)
			mon->s.skinnum = 2;

		//gi.dprintf ("%i %i %i\n", mon->health, mon->gib_health, mon->s.skinnum);

	}
	else
	{
		mon->name = "a Supertank";
		SP_monster_supertank(mon);
	}

	findspawnpoint(mon);
	mon->s.angles[1] = rand()%360;
	total_monsters++;

	//gi.dprintf ("A monster with mtype %i (%s) was spawned at %s\n", mtype, mon->name, vtos(mon->s.origin));

	gi.linkentity(mon);
	s->nextthink = level.time + 1;
}

void SpawnTheMonsterSpawnerMajiggy (void)
{
	edict_t *spawner;

	if (iwm_gamemode->value != 2)
		return;

	if (iwm_editor->value)
		return;

	total_monsters = 0;

	spawner = G_Spawn();
	spawner->think = SpawnAMonster;
	spawner->nextthink = level.time + 4;
	spawner->classname = "spawnmonsterer";

	gi.linkentity(spawner);
}

char *LoadEntFile(char *mapname, char *entities);
char *LoadEntFileOld(char *mapname, char *entities) ;

void SpawnNewEntities (char *mapname)
{
	edict_t		*ent;
	int			inhibit = 0;
	char		*com_token;
//	int			i;
//	float		skill_level;
	char *e;
//	int done = 0;
//	char *fix = "{\n";

	e = LoadEntFileOld(mapname, "null");
	//gi.dprintf ("%s\n", e);

	if (Q_stricmp(e, "null") == 0)
		return;
		
	// parse ents
	for (;;)
	{
		// parse the opening brace	
		com_token = COM_Parse (&e);
		if (!e)
			break;
		if (com_token[0] != '{')
			gi.error ("ED_LoadFromFile: found %s when expecting {",com_token);
		
		//if (!ent)
		//	ent = g_edicts;
		//else
			ent = G_Spawn ();

		e = ED_ParseEdict (e, ent);
		
		//gi.dprintf ("%s\n", e);
		
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
}

extern int is_demo;
void PrecacheMainModels (void);

int GetIndexValues ()
{
	int l = 0;
	int model = 0;
	int sound = 0;
	int pic = 0;

	for (l = 0; l < nummodelindexes; l++)
	{
		if (currentmodelindexes[l] == NULL)
			continue;
		if (currentmodelindexes[l][0] == 0)
			continue;
//		if (currentmodelindexes[l] == 0x0012f77c)
//			continue;
		model++;
	}
	for (l = 0; l < numsoundindexes; l++)
	{
		if (currentsoundindexes[l] == NULL)
			continue;
		if (currentsoundindexes[l][0] == 0)
			continue;
		if (!currentsoundindexes[l] || currentsoundindexes[l] == 0x00000000)
			continue;
		/*if (currentmodelindexes[l][0] == -18)
		{
			gi.dprintf ("Index %i: Unknown Sound (Probably a variable)\n", l);
			continue;
		}*/
		sound++;
	}
	// Paril: Are pics needed?
	for (l = 0; l < numimageindexes; l++)
	{
		if (currentimageindexes[l] == NULL)
			continue;
		if (currentimageindexes[l][0] == 0)
			continue;
//		if (currentmodelindexes[l] == 0x0012f77c)
//			continue;
		pic++;
	}
	return (model + sound + pic);
}

void EditorShowPaths ()
{
	edict_t	*ent;
	edict_t	*las;
//	int i;
	
	ent = NULL;
	gi.dprintf ("IWM Editor ===========================\nFunction: EditorShowPaths, Drawing path_corner Paths\n");
	while ((ent = G_Find (ent, FOFS(classname), "path_corner")))
	{
		if (ent->target)
		{
			las = G_Spawn();
			if (!las)
				break;
			VectorCopy(ent->laserer->s.origin, las->s.origin);
			las->target = ent->target;
			las->spawnflags = 33;
			las->spawnflags |= 1024;
			SP_target_laser (las);
		}
	}
	gi.dprintf ("Function: EditorShowPaths, Finished Drawing\n");
	gi.dprintf ("IWM Editor ===========================\n");
}

edict_t *firsthole, *lasthole;
int holes;

void SpawnEntities (char *mapname, char *entities, char *spawnpoint)
{
	edict_t		*ent;
	int			inhibit;
	char		*com_token;
	int			i;
	float		skill_level;
//	char *teh;
//	int done = 0;

	skill_level = floor (skill->value);
	if (skill_level < 0)
		skill_level = 0;
	if (skill_level > 3)
		skill_level = 3;
	if (skill->value != skill_level)
		gi.cvar_forceset("skill", va("%f", skill_level));

	entities_glb = entities;

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

	// Phlem - new for entity externally adding (and off-world teleport)
	//if (!custom_ents->value && !ctf->value)
	// CMO files.
	entities = LoadEntFile (mapname, entities);
	// end - new for entity externally adding

// parse ents
	for (;;)
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

	SpawnNewEntities(mapname);

//	CheckModelIndexes (NULL);
//	CheckSoundIndexes (NULL);

	if (!iwm_editor->value)
	{
		int get = GetIndexValues();

		gi.dprintf ("Get: %i\n", get);

		// 120 seems suitable
		if (get > 120)
			gi.dprintf ("************* " GAMEVERSION " *************\nWarning: This map holds over 120 indexes. Not indexing main models.\nSide effects: A small amount of lag.\nThis is done to prevent *Index Overflow errors, although in a bad way.\n************* " GAMEVERSION " *************\n");

	// Entities done
		if (deathmatch->value && get < 121)
			PrecacheMainModels();
	}

	if (iwm_editor->value)
	{
		EditorShowPaths ();
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

	// Paril Temp
	firsthole=NULL;
	lasthole=NULL;
	holes=0;

	if (!is_demo)
	{
		SpawnTheSpawner();
		SpawnTheMonsterSpawnerMajiggy();
	}

	level.dayoff = 1;


//	AI_NewMap();//JABot

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
// Watermark
//"if 18 "
"xv	40 "
"yb -32  "
"string2 \""VERSION"\" "	// VERSION defined in g_local.h
//"endif "

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

char *dm_statusbar =
// Watermark
//"if 31 "
"xv	40 "
"yb -32  "
"string2 \""VERSION"\"  "
//"endif "

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
"num 3 14 "

// chase camera
"if 16 "
  "xv 0 "
  "yb -68 "
  "string \"Chasing\" "
  "xv 64 "
  "stat_string 16 "
"endif "
;
char *snd[99];
char *names[103];

void PrecacheMainSounds (void)
{
	int i;

	snd[0] = "weapons/r_explode4.wav";
	snd[1] = "weapons/rocklx1a4.wav";
	snd[2] = "weapons/rballbounce.wav";
	snd[3] = "weapons/watermelonsplat.wav";
	snd[4] = "weapons/dodododoledowheee.wav";
	snd[5] = "weapons/lashit_.wav";
	snd[6] = "weapons/laserhit.wav";
	snd[7] = "weapons/waterballoon.wav";
	snd[8] = "weapons/bball.wav";
	snd[9] = "weapons/bounce.wav";
	snd[10] = "weapons/drill_loop.wav";
	snd[11] = "weapons/drill_warmdown.wav";
	snd[12] = "weapons/drill_warmup.wav";
	snd[13] = "weapons/light_hit.wav";
	snd[14] = "weapons/plasma.wav";
	snd[15] = "weapons/plasma_expl.wav";
	snd[16] = "weapons/light_warmup.wav";
	snd[17] = "weapons/devastator/boom.wav";
	snd[18] = "weapons/devastator/cooldown.wav";
	snd[19] = "weapons/devastator/fire.wav";
	snd[20] = "weapons/freezer/freeze.wav";
	snd[21] = "weapons/freezer/shatter1.wav";
	snd[22] = "weapons/rpod/rpodhit.wav";
	snd[23] = "weapons/rpod/rpodsho2.wav";
	snd[24] = "weapons/rpod/rpodshot.wav";
	snd[55] = "weapons/dkhammer.wav";
	snd[56] = "weapons/annoy.wav";
	snd[57] = "weapons/annoying.wav";
	snd[58] = "weapons/womd_in.wav";
	snd[59] = "weapons/art.wav";
	snd[60] = "weapons/iwm_expl.wav";
	snd[61] = "weapons/art_woosh.wav";
	snd[62] = "weapons/button.wav";
	snd[63] = "weapons/car_horn.wav";
	snd[64] = "weapons/disint2.wav";
	snd[65] = "weapons/disrupt.wav";
	snd[66] = "weapons/disrupthit.wav";
	snd[67] = "weapons/nail1.wav";
	snd[68] = "weapons/nukewarn2.wav";
	snd[69] = "weapons/phaloop.wav";
	snd[70] = "weapons/plasexpl.wav";
	snd[71] = "weapons/plasshot.wav";
	snd[72] = "weapons/proxopen.wav";
	snd[73] = "weapons/proxwarn.wav";
	snd[74] = "weapons/rippfire.wav";
	snd[75] = "weapons/sawhit.wav";
	snd[76] = "weapons/sawidle.wav";
	snd[77] = "weapons/tesla.wav";
	snd[78] = "weapons/teslaopen.wav";
	snd[79] = "weapons/trapcock.wav";
	snd[80] = "weapons/trapdown.wav";
	snd[81] = "weapons/traploop.wav";
	snd[82] = "weapons/trapsuck.wav";
	snd[83] = "weapons/stapler.wav";

	snd[84] = "";
	snd[85] = "";
	snd[86] = "";
	snd[87] = "";
	snd[88] = "weapons/pacman/pac_eat_50.wav";
	snd[89] = "";
	snd[90] = "";
	snd[91] = "";
	snd[92] = "";
	snd[93] = "";
	snd[94] = "weapons/pacman/pac_fly.wav";
	snd[95] = "";
	snd[96] = "weapons/pacman/pac_respawn.wav";
	snd[97] = "";

	snd[25] = "misc/gib1.wav";
	snd[26] = "misc/gib2.wav";
	snd[27] = "misc/gib3.wav";
	snd[28] = "misc/ir_start.wav";
	snd[29] = "misc/puddle_amb.wav";
	snd[30] = "misc/sheep1.wav";
	snd[31] = "misc/sheep2.wav";
	snd[30] = "misc/sheep3.wav";
	snd[31] = "misc/sheepdie.wav";

	snd[32] = "announcer/1frag.wav";
	snd[33] = "announcer/1min.wav";
	snd[34] = "announcer/2frag.wav";
	snd[35] = "announcer/2min.wav";
	snd[36] = "announcer/3frag.wav";
	snd[37] = "announcer/5min.wav";
	snd[38] = "announcer/10sec.wav";
	snd[39] = "announcer/20sec.wav";
	snd[40] = "announcer/accuracy.wav";
	snd[41] = "announcer/bluereturned.wav";
	snd[42] = "announcer/bluescores.wav";
	snd[43] = "announcer/blueteam.wav";
	snd[44] = "announcer/excellent.wav";
	snd[45] = "announcer/fraglimit.wav";
	snd[46] = "announcer/impressive.wav";
	snd[47] = "announcer/redreturned.wav";
	snd[48] = "announcer/redscores.wav";
	snd[49] = "announcer/redteam.wav";
	snd[50] = "announcer/theenemyhas.wav";
	snd[51] = "announcer/tie.wav";
	snd[52] = "announcer/timelimit.wav";
	snd[53] = "announcer/youhaveflag.wav";
	snd[54] = "announcer/yourteamhas.wav";

	snd[98] = "world/rumble.wav";
	snd[99] = "weapons/star.wav";

	for (i = 0; i < 99; i++)
	{
		SoundIndex (snd[i]);
	}
}

void PrecacheMainModels (void)
{
	int i;

	names[0] = "models/weapons/v_dev/tris.md2";
	names[1] = "models/weapons/v_rpod/tris.md2";
	names[2] = "";
	names[3] = "models/weapons/v_thor/tris.md2";
	names[4] = "models/weapons/v_drill/tris.md2";
	names[5] = "models/weapons/v_hose/tris.md2";
	names[6] = "models/weapons/v_lightn/tris.md2";
	names[38] = "models/weapons/v_needler/tris.md2";
	names[39] = "models/weapons/v_plasma/tris.md2";
	names[40] = "models/weapons/v_det/tris.md2";
	names[42] = "models/weapons/g_boom/tris.md2";
	names[43] = "";
	names[44] = "";
	names[45] = "";
	names[46] = "";
	names[47] = "";
	names[48] = "models/weapons/g_prox/tris.md2";
	names[49] = "";
	names[50] = "models/weapons/g_tesla/tris.md2";
	names[51] = "";
	names[52] = "models/weapons/v_beamer/tris.md2";
	names[53] = "models/weapons/v_beamer2/tris.md2";
	names[54] = "models/weapons/v_boomer/tris.md2";
	names[55] = "models/weapons/v_chainf/tris.md2";
	names[56] = "models/weapons/v_dist/tris.md2";
	names[57] = "models/weapons/v_etf_rifle/tris.md2";
	names[58] = "models/weapons/v_plaunch/tris.md2";
	names[59] = "models/weapons/v_shotx/tris.md2";
	names[60] = "models/weapons/v_tesla/tris.md2";
	names[61] = "models/weapons/v_tesla2/tris.md2";
	names[62] = "models/weapons/v_trap/tris.md2";
	names[65] = "models/weapons/z_trap/tris.md2";
	names[66] = "models/weapons/v_pig2k/tris.md2";
	names[67] = "models/weapons/v_stapler/tris.md2";

	names[68] = "models/items/flashlight/tris.md2";
	names[69] = "models/items/goggles/tris.md2";
	names[70] = "models/items/regen/tris.md2";
	names[71] = "models/items/regen/tris_outer.md2";
	names[72] = "models/items/dkh/tris.md2";
	names[73] = "models/items/boots/tris.md2";
	names[74] = "models/items/cloak/tris.md2";
	names[75] = "models/items/carpet/tris.md2";
	names[76] = "models/items/invis/inner.md2";
	names[77] = "models/items/invis/outer.md2";
	names[78] = "models/items/spawngro/tris.md2";
	names[103] = "models/items/star/tris.md2";

	names[7] = "models/proj/balloon/tris.md2";
	names[8] = "models/proj/bball/tris.md2";
	names[9] = "models/proj/beachball/tris.md2";
	names[10] = "models/proj/bean/tris.md2";
	names[11] = "models/proj/bomb/tris.md2";
	names[12] = "models/proj/bomb_spike/tris.md2";
	names[13] = "models/proj/disk/disk.md2";
	names[14] = "models/proj/disk/disk_bottom.md2";
	names[15] = "models/proj/disk/diskette.md2";
	names[16] = "models/proj/disintegrator/tris.md2";
	names[17] = "models/proj/jack/tris.md2";
	names[18] = "models/proj/lavaball/tris.md2";
	names[19] = "models/proj/lightning/tris.md2";
	names[20] = "models/proj/pacman/tris.md2";
	names[21] = "models/proj/plasma/tris.md2";
	names[22] = "models/proj/rball/tris.md2";
	names[23] = "models/proj/screwdriver/tris.md2";
	names[24] = "models/proj/firefly/tris.md2";
	names[25] = "models/proj/watermelon/tris.md2";
	names[36] = "models/proj/lavaball/tris.md2";
	names[37] = "models/proj/soundwave/tris.md2";
	names[79] = "models/proj/beam/tris.md2";
	names[80] = "models/proj/blade/tris.md2";
	names[81] = "models/proj/flechette/tris.md2";
	names[82] = "models/proj/ecks/tris.md2";
	names[83] = "models/proj/glowstick/tris.md2";
	names[84] = "models/proj/incend/tris.md2";
	names[85] = "models/proj/shard/tris.md2";
	names[87] = "models/proj/staple/tris.md2";
	names[88] = "models/proj/soundwave/tris.md2";
	names[90] = "models/proj/womd/tris.md2";
	names[100] = "models/proj/block/tris.md2";
	names[101] = "models/proj/rpod/tris.md2";
	names[102] = "models/proj/magnet/tris.md2";

	names[26] = "models/other/nuke/tris.md2";
	names[27] = "models/other/womd/tris.md2";
	names[28] = "models/other/cloud/tris.md2";
	names[29] = "models/other/raindrop/tris.md2";
	names[30] = "models/other/car/tris.md2";
	names[31] = "models/other/fire/fire.md2";
	names[32] = "models/other/gascloud/tris.md2";
	names[33] = "models/other/volcano/tris.md2";
	names[91] = "models/other/artillery/tris.md2";
	names[92] = "models/other/artillery/tris_base.md2";
	names[93] = "models/other/puddle/tris.md2";
	names[99] = "models/other/icec/tris.md2";
	names[64] = "models/objects/explode/tris.md2";
	names[41] = "models/objects/boomrang/tris.md2";
	names[35] = "models/objects/trapfx/tris.md2";
	names[89] = "models/objects/glass_gibs/gib1.md2";
	names[94] = "models/objects/glass_gibs/gib2.md2";
	names[95] = "models/objects/glass_gibs/gib3.md2";
	names[96] = "models/objects/glass_gibs/gib4.md2";
	names[97] = "models/objects/glass_gibs/gib5.md2";
	names[98] = "models/objects/dball/tris.md2";

	names[63] = "models/misc/sheep/tris.md2";

	names[34] = "sprites/null.sp2";

	for (i = 0; i < 103; i++)
	{
		ModelIndex (names[i]);
	}

	PrecacheMainSounds();
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

// Dibeggung.
gitem_t *weaponlist[MAX_ITEMS];


/*void Cmd_SeeWeaponsThingy (edict_t *ent)
{
	int i;

	gi.dprintf ("======================\n");
	for (i = 0; i < game.num_items; i++)
	{
		if (!weaponlist[i] || weaponlist[i] == NULL)
			continue;

		gi.dprintf ("%s - %i %i %i\n", weaponlist[i]->pickup_name, weaponlist[i]->bannumber, weaponlist[i]->weapnum, hex(weaponlist[i]->bannumber));
	}
	gi.dprintf ("======================\n");
}*/

// Paril, for bans, etc.
// Did this so I didn't have to mess with the itemlist.
// Hacks ahoy!
/*void InitializeWeaponNumbers (void)
{
	int i;
	int last_ban = 0;
	int last_num = 0;
	gitem_t *item;
	int w = 0;

	for (i = 0, item = itemlist ; i < game.num_items ; i++, item++)
	{
		// Not a weapon.
		// Revision: It adds armor, wtf.
		if (item->flags & IT_ARMOR)
			continue;
		if (!item->flags & IT_WEAPON)
			continue;

		// Pretty much all we need, let's go.
		// Okay, let's initialize the first ban number. It will be 1, of course.
		if (!last_ban && !last_num)
		{
			last_ban = last_num = 1;
			item->bannumber = last_ban;
			item->weapnum = last_num;
		}
		else
		{
			// Okay, it's more than one; this is a bit more complicated.
			// For the ban, we need to times the last ban number by two.
			// 1x2 = 2, 2x2 = 4, 4x2 = 8, 8x2 = 16, etc.
			item->bannumber = (last_ban * 2);
			item->weapnum = last_num + 1;

			last_ban *= 2;
			last_num += 1;
		}

		// Debugging.
		weaponlist[w] = item;
		w++;
	}
}*/

int is_demo;

void TurnW (edict_t *s)
{
	s->s.angles[1] += 15;

	s->nextthink = level.time + .1;
}

void wait ( int seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}


void SP_worldspawn (edict_t *ent)
{
	clock_t start;
	clock_t end;
	double elapsed;

	start = clock();
	//gi.dprintf ("%f\n", (float)start);

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
//ZOID
		if (ctf->value) {
			gi.configstring (CS_STATUSBAR, ctf_statusbar);
			//precaches
			ImageIndex("i_ctf1");
			ImageIndex("i_ctf2");
			ImageIndex("i_ctf1d");
			ImageIndex("i_ctf2d");
			ImageIndex("i_ctf1t");
			ImageIndex("i_ctf2t");
			ImageIndex("i_ctfj");
		} else
//ZOID
		gi.configstring (CS_STATUSBAR, dm_statusbar);
	else
		gi.configstring (CS_STATUSBAR, single_statusbar);

	//---------------


	// help icon for statusbar
	ImageIndex ("i_help");
	level.pic_health = ImageIndex ("i_health");
	ImageIndex ("help");
	ImageIndex ("field_3");

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "800");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	snd_fry = SoundIndex ("player/fry.wav");	// standing in lava / slime

	PrecacheItem (FindItem ("Blaster"));

	SoundIndex ("player/lava1.wav");
	SoundIndex ("player/lava2.wav");

	SoundIndex ("misc/pc_up.wav");
	SoundIndex ("misc/talk1.wav");

	SoundIndex ("misc/udeath.wav");

	// gibs
	SoundIndex ("items/respawn1.wav");

	// sexed sounds
	SoundIndex ("*death1.wav");
	SoundIndex ("*death2.wav");
	SoundIndex ("*death3.wav");
	SoundIndex ("*death4.wav");
	SoundIndex ("*fall1.wav");
	SoundIndex ("*fall2.wav");	
	SoundIndex ("*gurp1.wav");		// drowning damage
	SoundIndex ("*gurp2.wav");	
	SoundIndex ("*jump1.wav");		// player jump
	SoundIndex ("*pain25_1.wav");
	SoundIndex ("*pain25_2.wav");
	SoundIndex ("*pain50_1.wav");
	SoundIndex ("*pain50_2.wav");
	SoundIndex ("*pain75_1.wav");
	SoundIndex ("*pain75_2.wav");
	SoundIndex ("*pain100_1.wav");
	SoundIndex ("*pain100_2.wav");

	// sexed models
	// THIS ORDER MUST MATCH THE DEFINES IN g_local.h
	// you can add more, max 15
	ModelIndex ("#w_blaster.md2");
	ModelIndex ("#w_shotgun.md2");
	ModelIndex ("#w_sshotgun.md2");
	ModelIndex ("#w_machinegun.md2");
	ModelIndex ("#w_chaingun.md2");
	ModelIndex ("#a_grenades.md2");
	ModelIndex ("#w_glauncher.md2");
	ModelIndex ("#w_rlauncher.md2");
	ModelIndex ("#w_hyperblaster.md2");
	ModelIndex ("#w_railgun.md2");
	ModelIndex ("#w_bfg.md2");

	//-------------------

	SoundIndex ("player/gasp1.wav");		// gasping for air
	SoundIndex ("player/gasp2.wav");		// head breaking surface, not gasping

	SoundIndex ("player/watr_in.wav");	// feet hitting water
	SoundIndex ("player/watr_out.wav");	// feet leaving water

	SoundIndex ("player/watr_un.wav");	// head going underwater
	
	SoundIndex ("player/u_breath1.wav");
	SoundIndex ("player/u_breath2.wav");

	SoundIndex ("items/pkup.wav");		// bonus item pickup
	SoundIndex ("world/land.wav");		// landing thud
	SoundIndex ("misc/h2ohit1.wav");		// landing splash

	SoundIndex ("items/damage.wav");
	SoundIndex ("items/protect.wav");
	SoundIndex ("items/protect4.wav");
	SoundIndex ("weapons/noammo.wav");

	SoundIndex ("infantry/inflies1.wav");

	sm_meat_index = ModelIndex ("models/objects/gibs/sm_meat/tris.md2");
	ModelIndex ("models/objects/gibs/arm/tris.md2");
	ModelIndex ("models/objects/gibs/bone/tris.md2");
	ModelIndex ("models/objects/gibs/bone2/tris.md2");
	ModelIndex ("models/objects/gibs/chest/tris.md2");
	ModelIndex ("models/objects/gibs/skull/tris.md2");
	ModelIndex ("models/objects/gibs/head2/tris.md2");



//
// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.
//

	// 0 normal
	gi.configstring(CS_LIGHTS+0, "a");
	
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
	// Paril: was 'a'
	gi.configstring(CS_LIGHTS+63, "a");

//	InitializeWeaponNumbers();

	level_gibs = 0;

	// Is this game a demo? Need to do this for a few entities.
	{
		char *pch;

		pch = strstr (level.mapname, ".dm2");
		if (pch != NULL)
			is_demo = 1;
		else
			is_demo = 0;
	}

	end = clock();
	//gi.dprintf ("%f\n", (float)end);
	elapsed = ((double) (end - start) / CLOCKS_PER_SEC);

	gi.dprintf ("Worldspawn initialized in %f seconds.\n", elapsed);
}

