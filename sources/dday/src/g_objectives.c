/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/g_objectives.c,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/06/04 19:49:46 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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
//#include "p_menus.h"

// g_objectives.c
// D-Day: Normandy Objective Entities

#if 0

	char *obj_name;
	float obj_area;
	float obj_time;
	int	obj_owner;	//entity that owns this item
	int	obj_gain;
	int	obj_loss;
	int obj_count;

#endif // 0

/*
========================
objective_area
========================
*/
void objective_area_think (edict_t *self) {

	edict_t *ent  = NULL;
	int count = 0;
	int i=0;
	int newteam;
	int delay;

	self->nextthink = level.time + FRAMETIME;

	if (self->delay) // if there's a counter running
	{
	}

	while ((ent = findradius(ent, self->s.origin, self->obj_area)) != NULL)
	{
		if (!ent->inuse)
			continue;
		if (!IsValidPlayer(ent))
			continue;

		newteam = ent->client->resp.team_on->index;

		if (newteam != self->obj_owner)
			count++;

		//gi.dprintf("Found %d players\n", count);		
	}

	if (count >= self->obj_count) {

		team_list[self->obj_owner]->score -= self->obj_loss;

		self->obj_owner = team_list[newteam]->index;
		team_list[self->obj_owner]->score += self->obj_gain;

		if (team_list[self->obj_owner]->time_to_win) // If there already is a counter somwhere else
		{
			if (team_list[self->obj_owner]->time_to_win > (self->obj_time + level.time) )
			// If the counter is longer, shorten it up to this one
				team_list[self->obj_owner]->time_to_win = (self->obj_time + level.time);
		} else // there is no counter
			team_list[self->obj_owner]->time_to_win = (self->obj_time + level.time);

		delay = (int)(team_list[self->obj_owner]->time_to_win - level.time);

		if ((delay/60) >= 1)
			gi.bprintf(PRINT_HIGH, "Team %s has %i minutes before they win the battle.\n", team_list[self->obj_owner]->teamname, (delay/60));
		else
			gi.bprintf(PRINT_HIGH, "Team %s has %i seconds before they win the battle.\n", team_list[self->obj_owner]->teamname, delay);

		gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/area_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);

		if (dedicated->value)
			gi.cprintf(NULL, PRINT_HIGH, "Objective %s taken by team %s!\n",  self->obj_name,  team_list[self->obj_owner]->teamname);

		centerprintall("Objective %s taken\n by team %s!\n", 
			self->obj_name, 
			team_list[self->obj_owner]->teamname);
	}
}

void SP_objective_area(edict_t *self) {	

	if (!self->obj_name)
		 self->obj_name = "Objective";
	if (!self->obj_area)
		 self->obj_area = 100.0;
//	if (!self->obj_time)
		 self->obj_time = 120;
	if (!self->obj_count)
		 self->obj_count = 3;

	gi.dprintf("\n\nobjective_area spawned belonging to team %i (%s) as \"%s\"\n",
		self->obj_owner,
        team_list[self->obj_owner]->teamname,
        self->obj_name);

	gi.dprintf("distance: %f\n", 
		self->obj_area);

	gi.dprintf("award: %i, loss: %i\n", 
		self->obj_gain,
		self->obj_loss);
	   
	gi.dprintf("required persons: %i\n", self->obj_count);
	gi.dprintf("must hold for %i seconds.\n\n",	(int)self->obj_time);
	
	gi.dprintf(" mins: %s\n maxs: %s\n\n",
		vtos(self->mins),
		vtos(self->maxs) );

	self->think=objective_area_think;	
	self->nextthink = level.time + FRAMETIME;

	self->movetype = MOVETYPE_NONE;
//	self->svflags |= SVF_NOCLIENT;
	gi.setmodel (self, self->model);
	self->solid = SOLID_NOT;
	gi.linkentity (self);

	gi.dprintf(" mins: %s\n maxs: %s\n\n",
		vtos(self->mins),
		vtos(self->maxs) );

}


/*
========================
objective_touch
========================
*/
void objective_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {

	//edict_t *entC = NULL;

	if (!IsValidPlayer(other) || (other->client->resp.mos == MEDIC && invuln_medic->value == 1) )  
		return;

	//gi.dprintf("touch %i:%i (%i)\n", level.framenum, self->obj_count, (level.framenum - self->obj_count));

	if (other->client->resp.team_on->index != self->obj_owner) 
	{
		if ((level.framenum - self->obj_count) <= 15) // its been at least a frame since own team touched it
			return;

		if (self->obj_owner < MAX_TEAMS) // undefined teams
			team_list[self->obj_owner]->score -= self->dmg;
		
		self->obj_owner = other->client->resp.team_on->index;
		team_list[self->obj_owner]->score += self->health;

		gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);

		if (dedicated->value)
			gi.cprintf(NULL, PRINT_HIGH, "%s taken by %s [%s]\n", 
				self->message, 
				other->client->pers.netname,
				team_list[self->obj_owner]->teamname);

		centerprintall("%s taken by:\n\n%s\n%s",
				self->message, 
				other->client->pers.netname,
				team_list[self->obj_owner]->teamname);
		
		self->obj_count = level.framenum; // reset the touch count
	} 
	else  // own team touched it
	{
		//gi.dprintf("%s deadflag: %i\n", other->client->pers.netname, other->deadflag);

		if (other->deadflag == DEAD_NO)
			self->obj_count = level.framenum; // update the last time team touched it
	}

}

/// exactly like the one above, except 1 person needs to touch the thing.
void SP_objective_touch(edict_t *self) {	

	self->touch=objective_touch;
	//self->index=st.obj_owner;
	self->movetype = MOVETYPE_NONE;	
	self->solid = SOLID_TRIGGER;
	gi.setmodel (self, self->model);	
	gi.linkentity (self);
	
}


/*
========================
func_explosive_objective
========================
*/
void func_explosive_objective_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;
	int		enemy;

	//gi.dprintf("self: %s\ninflictor: %s\n attacker: %s\n",
	//	self->classname, inflictor->classname, attacker->classname);

	if (!attacker->client ||
		!attacker->client->resp.mos)
		return;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize (self->velocity);
	VectorScale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	// hack for 2 team games

	if (self->obj_owner != 99) {
		team_list[self->obj_owner]->score -= self->obj_loss;
		enemy = (self->obj_owner) ? 0 : 1;
	} else
		enemy = 99;

	if (self->obj_owner != attacker->client->resp.team_on->index)
		team_list[attacker->client->resp.team_on->index]->score += self->obj_gain;
	else if (self->obj_owner == attacker->client->resp.team_on->index && enemy != 99)
		team_list[enemy]->score += self->obj_gain;

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_HIGH, "%s destroyed by %s [%s]\n", 
			self->obj_name, 
			attacker->client->pers.netname,
			team_list[attacker->client->resp.team_on->index]->teamname);

	centerprintall("%s destroyed by:\n\n%s\n%s",
		self->obj_name, 
		attacker->client->pers.netname,
		team_list[attacker->client->resp.team_on->index]->teamname);

	gi.sound(self, CHAN_NO_PHS_ADD, gi.soundindex(va("%s/objectives/touch_cap.wav", team_list[self->obj_owner]->teamid)), 1, 0, 0);

	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);
}

void func_explosive_objective_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_objective_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_objective_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

void SP_func_explosive_objective (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_objective_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_objective_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_objective_use)
	{
		if (!self->health)
			self->health = 500;
		self->die = func_explosive_objective_explode;
		self->takedamage = DAMAGE_YES;
	}

	if (!self->obj_name)
		self->obj_name = "Objective";
	if (!self->obj_gain)
		self->obj_gain = 5;
//	if (!self->obj_loss)
//		self->obj_loss = 5;

	gi.linkentity (self);
}

void GetMapObjective (void) {

	FILE *map_file;
	char filename[100];
	
	strcpy(filename, GAMEVERSION "/pics/objectives/");		
	strcat(filename, level.mapname);
	strcat(filename,".pcx");

	gi.dprintf("Loading map objective pic %s...", filename);
	if (map_file = fopen(filename, "r")) 
	{
		fclose(map_file);
		level.objectivepic = filename;
		gi.dprintf("done.\n");
	} 
	else
		gi.dprintf("error.\n");
}

