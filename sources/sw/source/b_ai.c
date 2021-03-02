#include "g_local.h"
#include "m_player.h"
#include "b_local.h"

/*
Find_Nearest_Node(edict_s * 0xcccccccc) line 24 + 14 bytes
Bot_Find_Roam_Goal(edict_s * 0x032a178c) line 725 + 9 bytes

Variables:
self->s.origin[0] through self->s.origin[2] cannot be evaluated.
*/

float Bot_Calc_Path_Cost (counter_t start, counter_t end)
{
	int i;
	counter_t node;

	i = 0;

	if(start == NO_NODES || end == NO_NODES)
		return NO_NODES;

	node = start;

	while(i < MAX_NODES)
	{
		if(node == end)
		{
			return i;
		}
		else if (graph[node].dist[end] < INFINITY)
		{
			node = graph[node].last[end];
		}
		else if (Dijkstra_ShortestPath (node, end))
		{
			node = graph[node].last[end];
		}
		else
		{
			return NO_NODES;
		}
		i++;
	}
	return NO_NODES;
}

float Bot_Find_Item_Weight (item_table_t *item)
{
	if (item->item_index == ITEM_AMMO_DL44)
		return WEIGHT_AMMO_DL44;
	else if (item->item_index == ITEM_AMMO_E11)
		return WEIGHT_AMMO_E11;
	else if (item->item_index == ITEM_AMMO_T21)
		return WEIGHT_AMMO_T21;
	else if (item->item_index == ITEM_AMMO_DISRUPTOR)
		return WEIGHT_AMMO_DISRUPTOR;
	else if (item->item_index == ITEM_AMMO_BOLTS)
		return WEIGHT_AMMO_BOLTS;
	else if (item->item_index == ITEM_AMMO_CMISS)
		return WEIGHT_AMMO_CMISS;
	else if (item->item_index == ITEM_AMMO_BMISS)
		return WEIGHT_AMMO_BMISS;
	else if (item->item_index == ITEM_AMMO_BACKPACK)
		return WEIGHT_AMMO_BACKPACK;
	else if (item->item_index == ITEM_AMMO_SNIPER)
		return WEIGHT_AMMO_SNIPER;
	else if (item->item_index == ITEM_WEAPON_THERMAL)
		return WEIGHT_WEAPON_THERMAL;
	else if (item->item_index == ITEM_WEAPON_PISTOL)
		return WEIGHT_WEAPON_PISTOL;
	else if (item->item_index == ITEM_WEAPON_RIFLE)
		return WEIGHT_WEAPON_RIFLE;
	else if (item->item_index == ITEM_WEAPON_REPEATER)
		return WEIGHT_WEAPON_REPEATER;
	else if (item->item_index == ITEM_WEAPON_BOWCASTER)
		return WEIGHT_WEAPON_BOWCASTER;
	else if (item->item_index == ITEM_WEAPON_WRISTROCKET)
		return WEIGHT_WEAPON_WRISTROCKET;
	else if (item->item_index == ITEM_WEAPON_MISSILETUBE)
		return WEIGHT_WEAPON_MISSILETUBE;
	else if (item->item_index == ITEM_WEAPON_DISRUPTOR)
		return WEIGHT_WEAPON_DISRUPTOR;
	else if (item->item_index == ITEM_WEAPON_SNIPER)
		return WEIGHT_WEAPON_SNIPER;
	else if (item->item_index == ITEM_WEAPON_BEAMTUBE)
		return WEIGHT_WEAPON_BEAMTUBE;
//	else if (item->item_index == ITEM_ARMOR_JACKET)
//		return WEIGHT_ARMOR_JACKET;
//	else if (item->item_index == ITEM_ARMOR_COMBAT)
//		return WEIGHT_ARMOR_COMBAT;
//	else if (item->item_index == ITEM_ARMOR_BODY)
//		return WEIGHT_ARMOR_BODY;
//	else if (item->item_index == ITEM_ARMOR_SHARD)
//		return WEIGHT_ARMOR_SHARD;
	else
		return 0;
}


qboolean Bot_Needs_Item (edict_t *self, item_table_t *item)
{
	int			max;

	if (item->item_index == ITEM_AMMO_DL44)
		max = CLIP_PISTOL;
	else if (item->item_index == ITEM_AMMO_E11)
		max = CLIP_RIFLE;
	else if (item->item_index == ITEM_AMMO_T21)
		max = CLIP_REPEATER;
	else if (item->item_index == ITEM_AMMO_DISRUPTOR)
		max = CLIP_DISRUPTOR;
	else if (item->item_index == ITEM_AMMO_BOLTS)
		max = CLIP_BOWCASTER;
	else if (item->item_index == ITEM_AMMO_CMISS)
		max = CLIP_MISSILETUBE;
	else if (item->item_index == ITEM_AMMO_BMISS)
		max = CLIP_WRISTROCKET;
	else if (item->item_index == ITEM_AMMO_BACKPACK)
		max = CLIP_BEAMTUBE;
	else if (item->item_index == ITEM_AMMO_SNIPER)
		max = CLIP_SNIPER;
	else if (item->item_index == ITEM_WEAPON_THERMAL)
		max = 10;
	else if (item->item_index == ITEM_WEAPON_PISTOL)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_RIFLE)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_REPEATER)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_BOWCASTER)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_WRISTROCKET)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_MISSILETUBE)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_DISRUPTOR)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_SNIPER)
		max = 5;
	else if (item->item_index == ITEM_WEAPON_BEAMTUBE)
		max = 5;
//	else if (item->item_index == ITEM_ARMOR_JACKET)
//		max = 200;
//	else if (item->item_index == ITEM_ARMOR_COMBAT)
//		max = 150;
//	else if (item->item_index == ITEM_ARMOR_BODY)
//		max = 100;
	else
		return false;

	if (self->client->pers.inventory[item->item_index] >= max)
		return false;

	return true;
}

qboolean Bot_ChangeWeapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// if we don't have any ammo for the weapon, return
	if (item->ammo)
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		if(!ent->client->pers.clipammo[ITEM_INDEX(item)])
		{
			if (!ent->client->pers.inventory[ammo_index])
			{
				return false;
			}
		}
	}

	// check to see if the bot is already using the weapon
	if (item == ent->client->pers.weapon)
	{

		// if we've got ammo, keep the current weapon
		return true;
	}

	// return if the bot doesn't have the weapon in inventory
	if(!ent->client->pers.inventory[ITEM_INDEX(item)])
		return false;

	ent->client->newweapon = item;

	return true;
}

void Bot_Choose_Weapon (edict_t *self)
{
	if(saberonly->value)
	{
		Bot_ChangeWeapon(self, FindItem("Lightsaber"));
		return;
	}

	if(skill->value >= 3 && Bot_ChangeWeapon(self, FindItem("Thermals")))
	{
		return;
	}

	if(skill->value >= 3 && Bot_ChangeWeapon(self, FindItem("Rocket_Launcher")))
	{
		return;
	}
	
	if(skill->value >= 3 && Bot_ChangeWeapon(self, FindItem("Beam_Tube")))
	{
		return;
	}

	if(skill->value >= 3 && Bot_ChangeWeapon(self, FindItem("Night_Stinger")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Disruptor")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Wrist_Rocket")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Bowcaster")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Repeater")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Trooper_Rifle")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Blaster")))
	{
		return;
	}

	if(Bot_ChangeWeapon(self, FindItem("Lightsaber")))
	{
		return;
	}
}

void Bot_Attack (edict_t *self, edict_t *other, usercmd_t *ucmd)
{
	float		x, y;
	float		r, dist;
	qboolean	swing;
	vec3_t	v;

	VectorSubtract (other->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);

	self->s.angles[YAW] = self->ideal_yaw;

	r = v[PITCH]*v[PITCH]+v[YAW]*v[YAW];
	x = sqrt(r);
	y = v[ROLL];

	self->s.angles[PITCH] = -atan2(y, x) * 180 / M_PI;
	if(self->s.angles[PITCH] < 0)
	self->s.angles[PITCH] += 360;

	//gi.error("cow\n");

	if(self->client->pers.weapon == FindItem("Lightsaber"))
	{
		if(self->sflags & MOVE_DUEL_STRIKE && random() > 0.2)
		{
			swing = 1;
		}
		else if (random() > 0.8)
		{
			swing = 1;
		}
		if(other->sflags & MOVE_DUEL_STRIKE && other->is_bot)
		{
			swing = 0;
		}
		r = random();
		dist = VectorLengthSquared(v);

/*		if(self->client->weaponstate != WEAPON_FIRING && other->client->weaponstate == WEAPON_FIRING && other->client->pers.weapon == FindItem("Lightsaber") && dist <= 4096)
		{
			if(other->client->swing_num == SWING_CHOP || other->client->swing_num == SWING_THRUST)
			{
				ucmd->forwardmove = -100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(other->client->swing_num == SWING_UCUTLEFT || other->client->swing_num == SWING_UCUTRIGHT)
			{
				ucmd->forwardmove = -100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(other->client->swing_num == SWING_RIGHT || other->client->swing_num == SWING_SLICERIGHT)
			{
				ucmd->forwardmove = -100;
				ucmd->sidemove = 100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(other->client->swing_num == SWING_LEFT || other->client->swing_num == SWING_SLICELEFT)
			{
				ucmd->forwardmove = -100;
				ucmd->sidemove = -100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
		}
		else*/ if(self->client->weaponstate == WEAPON_FIRING)
		{
			if(self->client->swing_num == SWING_CHOP)
			{
				ucmd->forwardmove = 0;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == SWING_RIGHT)
			{
				ucmd->sidemove = -100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == SWING_LEFT)
			{
				ucmd->sidemove = 100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == SWING_SLICERIGHT)
			{
				ucmd->upmove = -400;
				ucmd->sidemove = -100;
				ucmd->forwardmove = -100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == SWING_SLICELEFT)
			{
				ucmd->upmove = -400;
				ucmd->sidemove = 100;
				ucmd->forwardmove = -100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == SWING_THRUST)
			{
				ucmd->forwardmove = 400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == BLOCK_UP)
			{
				ucmd->forwardmove = -100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == BLOCK_LOW)
			{
				ucmd->forwardmove = -100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == BLOCK_LEFT)
			{
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(self->client->swing_num == BLOCK_RIGHT)
			{
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else
			{
				ucmd->forwardmove = 400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
		}
		else if(dist <= 4096 && swing && other->client->weaponstate != WEAPON_FIRING)
		{
			if(r > 0.8) //CHOP
			{
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(r > 0.6) //RIGHT
			{
				ucmd->sidemove = -100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(r > 0.4) //LEFT
			{
				ucmd->sidemove = 100;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(r > 0.3) //SLICE RIGHT
			{
				ucmd->sidemove = -100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else if(r > 0.2) //SLICE LEFT
			{
				ucmd->sidemove = 100;
				ucmd->upmove = -400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			else //THRUST
			{
				ucmd->forwardmove = 400;
				ucmd->buttons |= BUTTON_ATTACK;
			}
			self->sflags |= MOVE_DUEL_STRIKE;
		}
		else if (dist <= 4096)
		{
			self->sflags &= ~MOVE_DUEL_STRIKE;
			ucmd->forwardmove = -200;
		}
		else if (dist <= 16384 && random() > 0.1)
		{
			self->sflags &= ~MOVE_DUEL_STRIKE;

			if(self->sflags & MOVE_DUEL_RIGHT)
			{
				ucmd->sidemove = 50;
				if(random() > 0.9)
				{
					self->sflags &= ~MOVE_DUEL_RIGHT;
				}
			}
			else if(self->sflags & MOVE_DUEL_LEFT)
			{
				ucmd->sidemove = -50;
				if(random() > 0.9)
				{
					self->sflags &= ~MOVE_DUEL_LEFT;
				}
			}
			else
			{
				self->sflags |= MOVE_DUEL_HOLD;
				if(random() > 0.9)
				{
					if(random() > 0.5)
					{
						self->sflags |= MOVE_DUEL_RIGHT;
						self->sflags &= ~MOVE_DUEL_HOLD;
					}
					else
					{
						self->sflags |= MOVE_DUEL_LEFT;
						self->sflags &= ~MOVE_DUEL_HOLD;
					}
				}
			}
		}
		else
		{
			self->sflags &= ~MOVE_DUEL_STRIKE;
			ucmd->forwardmove = 400;
		}
	}
	else
	{
		r = random();
		if(r < 0.2)
			ucmd->sidemove = -400;
		else if(r < 0.4)
			ucmd->sidemove = 400;
		else if(r < 0.65)
			ucmd->forwardmove = -400;
		else if(r < 0.9)
			ucmd->forwardmove = 400;
		else if(r < 0.95)
			ucmd->upmove = -400;
		else if(r < 1.0)
			ucmd->upmove = 400;
		ucmd->buttons |= BUTTON_ATTACK;
	}
}

void Bot_Move (edict_t *self, usercmd_t *ucmd)
{
	if(VectorLengthSquared(self->velocity) < 4096 && !self->enemy) //make this check for distance to enemy instead of just enemy
	{
		self->s.angles[YAW] += random() * 360 - 180; 
		ucmd->forwardmove = -400;

		return;
	}
	else if(strcmp(self->client->pers.weapon->classname, "weapon_saber") == 0)
	{
		ucmd->forwardmove = 200;
		return;
	}

	if(random() > 0.99)
		ucmd->upmove = 400;

	ucmd->forwardmove = 400;
}

counter_t Bot_Find_Nearest_Node (edict_t *self, counter_t ignore)
{
	counter_t		start_node, i;
	distance_t		dist, best_dist;
	vec3_t	dir;
	trace_t	tr;
	int		content_mask;

	if((unsigned int)self == 0xcccccccc)
	{
		return NO_NODES;
	}

	if(!self)
	{
		return NO_NODES;
	}

	content_mask = CONTENTS_SOLID | CONTENTS_WINDOW;

	best_dist = (distance_t)MAX_LINK_DIST_SQUARED;

	start_node = NO_NODES;
	// search for nearest node to nodetest ent
	for (i = 0; i < node_count; i++)		
	{
		if(i == ignore)
			continue;

//	RipVTide - This seems to be the line causing the bots crashing
//	ToDo

		VectorSubtract (self->s.origin, node_list[i], dir);
		dist = (distance_t)VectorLengthSquared(dir);

		if (dist > MAX_NODE_DIST_SQUARED + MAX_NODE_DIST_SQUARED)
			continue;

		//tr = gi.trace (start, vec3_origin, vec3_origin, end, self, MASK_OPAQUE);
		if (dist < best_dist)
		{
			tr = gi.trace (self->s.origin, vec3_origin, vec3_origin, node_list[i], self, content_mask);

			if(tr.fraction == 1)
			{
				start_node = i;
				best_dist = dist;
			}
		}
	}
	return start_node;
}


void Bot_Check_Node_Dist (edict_t *self)
{
	vec3_t	v;

	if(self->path_node == NO_NODES || self->moveflags & MOVE_ITEM)
		return;

	VectorSubtract (self->s.origin, node_list[self->path_node], v);
	if(VectorLengthSquared(v) < 1024)
	{
		Bot_Find_Roam_Goal(self);
		self->last_path_node = self->path_node;
		self->path_node = NO_NODES;
	}
}

void Bot_Turn_To_Goal (edict_t *self)
{
	vec3_t	v;

	if(self->moveflags & MOVE_ITEM)
	{
		VectorSubtract (self->target_ent->s.origin, self->s.origin, v);
	}
	else
	{
		VectorSubtract (node_list[self->path_node], self->s.origin, v);
	}
	self->ideal_yaw = vectoyaw(v);

	self->s.angles[YAW] = self->ideal_yaw;
//	M_ChangeYaw(self);
}

int Bot_Find_Enemy (edict_t *self)
{
	int	i;
	float	weight, best_weight;
	float	dist;
	vec3_t	v;

	best_weight = 0;

	for(i=0;i<num_players;i++)
	{
		if(players[i] == self)
			continue;

		if(players[i]->flags & FL_NOTARGET)
			continue;

		if(ctf->value)
		{
			if(self->client->resp.ctf_team == players[i]->client->resp.ctf_team)
				continue;
		}

		if((Force_constant_active(players[i], LFORCE_INVISIBILITY) != 255) && players[i]->client->invisi_time < level.time)
			continue;

		if(players[i]->deadflag)
			continue;

		VectorSubtract(self->s.origin, players[i]->s.origin, v);
		dist = VectorLengthSquared(v);

		weight = 1/dist;

		if(weight > best_weight)
		{
			if(visible(self, players[i]) && gi.inPVS (self->s.origin, players[i]->s.origin))
			{
				best_weight = weight;
				self->enemy = players[i];
			}
		}
	}

	if(self->enemy)
	{
		if(!saberonly->value)
			self->moveflags &= ~MOVE_ITEM;
		return 1;
	}

	return 0;
}

void Bot_Find_Roam_Goal (edict_t *self)
{
	counter_t	node, far_node, near_node;
	float		weight, best_weight, cost, newweight;
	edict_t	*ent;
	int		i, playergoal, itemgoal;

	if(self->path_node != NO_NODES && level.time < self->check_path_time)
	{
		return;
	}

	if(self->moveflags & MOVE_ITEM)
	{
		if(self->target_ent->solid == SOLID_NOT)
		{
			self->target_ent = NULL;
			self->moveflags &= ~MOVE_ITEM;
		}
		else
		{
			return;
		}
	}

	if(level.time > self->check_path_time)
	{
		node = Bot_Find_Nearest_Node(self, self->last_path_node);
//		node = Find_Nearest_Node(self);
		self->moveflags |= MOVE_ROAM;
		self->check_path_time = level.time + 2 + random();
		self->path_node = node;

		return;
	}

	near_node = Bot_Find_Nearest_Node (self, NO_NODES);
//	near_node = Find_Nearest_Node (self);

	if(near_node == NO_NODES)
		return;

	best_weight = 0;

	if(!saberonly->value)
	for(i=0;i<num_items;i++)
	{
		if(!item_table[i].ent->classname)
			continue;
		if(item_table[i].ent->solid == SOLID_NOT)
			continue;

		if(!Bot_Needs_Item (self, &item_table[i]))
			continue;

		weight = Bot_Find_Item_Weight (&item_table[i]);

		if(!weight)
			continue;

		if(item_table[i].node == NO_NODES)
			continue;

		cost = Bot_Calc_Path_Cost (near_node, item_table[i].node);

		if(cost == NO_NODES)
			continue;

		newweight = (1/cost)*weight;

		if(newweight > best_weight)
		{
			best_weight = newweight;
			ent = item_table[i].ent;
			itemgoal = i;
		}
	}

	if(!self->oldenemy)
	{
		for(i=0;i<num_players;i++)
		{
			if(!players[i] || !players[i]->inuse)
				continue;

			if(players[i] == self)
				continue;

			if(players[i]->flags & FL_NOTARGET)
				continue;

			if(players[i]->deadflag)
				continue;

			if((Force_constant_active(players[i], LFORCE_INVISIBILITY) != 255) && players[i]->client->invisi_time < level.time)
				continue;

			far_node = Bot_Find_Nearest_Node (players[i], NO_NODES);
//			far_node = Find_Nearest_Node (players[i]);

			if(far_node == NO_NODES)
				continue;

			cost = Bot_Calc_Path_Cost (near_node, far_node);

			if(cost == NO_NODES)
				continue;

			newweight = (1/cost)*50;

			if(newweight > best_weight)
			{
				best_weight = newweight;
				ent = players[i];
				playergoal = 1;
			}
		}
	}
	else
	{
		self->enemy = self->oldenemy;
	}
	if(!ent)
		return;

	if(playergoal)
	{
		far_node = Bot_Find_Nearest_Node (ent, NO_NODES);
//		far_node = Find_Nearest_Node (ent);
		self->moveflags &= ~MOVE_ITEM;
	}
	else if (itemgoal)
	{
		far_node = item_table[itemgoal].node;
	}

	if(near_node == NO_NODES || far_node == NO_NODES)
		return;

	if(self->last_path_node == far_node && !playergoal)
	{
		self->moveflags |= MOVE_ITEM;
		self->target_ent = ent;
		return;
	}

	if(near_node == far_node)
	{
		node = near_node;
	}
	else if (graph[near_node].dist[far_node] < INFINITY)
	{
		// path already calculated before
		node = graph[near_node].last[far_node];
	}
	else if (Dijkstra_ShortestPath (near_node, far_node))
	{
		node = graph[near_node].last[far_node];
	}
	else
	{
		return;
	}

	self->moveflags |= MOVE_ROAM;
	self->check_path_time = level.time + 3 + random();
	self->path_node = node;
}

void Bot_CTF_Think (edict_t *self)
{
	counter_t	node, far_node, near_node;
	float		weight, best_weight, cost, newweight;
	edict_t	*ent;
	int		i, playergoal, itemgoal;

	if(self->path_node != NO_NODES && level.time < self->check_path_time)
	{
		return;
	}
	if(self->moveflags & MOVE_ITEM)
	{
		if(self->target_ent->solid == SOLID_NOT)
		{
			self->target_ent = NULL;
			self->moveflags &= ~MOVE_ITEM;
		}
		else
		{
			return;
		}
	}

	if(level.time > self->check_path_time)
	{
		node = Bot_Find_Nearest_Node(self, self->last_path_node);
//		node = Find_Nearest_Node(self);
		self->moveflags |= MOVE_ROAM;
		self->check_path_time = level.time + 2 + random();
		self->path_node = node;

		return;
	}

	near_node = Bot_Find_Nearest_Node (self, NO_NODES);
//	near_node = Find_Nearest_Node (self);

	if(near_node == NO_NODES)
		return;

	best_weight = 0;

	if(!saberonly->value)
	{
		for(i=0;i<num_items;i++)
		{
			if(!item_table[i].ent->classname)
				continue;
			if(item_table[i].ent->solid == SOLID_NOT)
				continue;
	
			if(!Bot_Needs_Item (self, &item_table[i]))
				continue;	
	
			weight = Bot_Find_Item_Weight (&item_table[i]);

			if(!weight)
				continue;

			if(item_table[i].node == NO_NODES)
				continue;

			cost = Bot_Calc_Path_Cost (near_node, item_table[i].node);
	
			if(cost == NO_NODES)
				continue;

			newweight = (1/cost)*weight;

			if(newweight > best_weight)
			{
				best_weight = newweight;
				ent = item_table[i].ent;
				itemgoal = i;
			}
		}
	}

	if(!self->oldenemy)
	{
		for(i=0;i<num_players;i++)
		{
			if(!players[i] || !players[i]->inuse)
				continue;

			if(players[i] == self)
				continue;

			if(players[i]->flags & FL_NOTARGET)
				continue;

			if(players[i]->deadflag)
				continue;

			if((Force_constant_active(players[i], LFORCE_INVISIBILITY) != 255) && players[i]->client->invisi_time < level.time)
				continue;

			far_node = Bot_Find_Nearest_Node (players[i], NO_NODES);
//			far_node = Find_Nearest_Node (players[i]);

			if(far_node == NO_NODES)
				continue;

			cost = Bot_Calc_Path_Cost (near_node, far_node);

			if(cost == NO_NODES)
				continue;

			newweight = (1/cost)*50;

			if(newweight > best_weight)
			{
				best_weight = newweight;
				ent = players[i];
				playergoal = 1;
			}
		}
	}
	else
	{
		self->enemy = self->oldenemy;
	}
	if(!ent)
		return;

	if(playergoal)
	{
		far_node = Bot_Find_Nearest_Node (ent, NO_NODES);
//		far_node = Find_Nearest_Node (ent);
		self->moveflags &= ~MOVE_ITEM;
	}
	else if (itemgoal)
	{
		far_node = item_table[itemgoal].node;
	}

	if(near_node == NO_NODES || far_node == NO_NODES)
		return;

	if(self->last_path_node == far_node && !playergoal)
	{
		self->moveflags |= MOVE_ITEM;
		self->target_ent = ent;
		return;
	}

	if(near_node == far_node)
	{
		node = near_node;
	}
	else if (graph[near_node].dist[far_node] < INFINITY)
	{
		// path already calculated before
		node = graph[near_node].last[far_node];
	}
	else if (Dijkstra_ShortestPath (near_node, far_node))
	{
		node = graph[near_node].last[far_node];
	}
	else
	{
		return;
	}

	self->moveflags |= MOVE_ROAM;
	self->check_path_time = level.time + 3 + random();
	self->path_node = node;

}

void Bot_AI_Think (edict_t *self)
{
	usercmd_t	ucmd;

	// Set up client movement
	VectorCopy(self->client->ps.viewangles,self->s.angles);
	VectorSet (self->client->ps.pmove.delta_angles, 0, 0, 0);
	memset (&ucmd, 0, sizeof (ucmd));
	self->oldenemy = self->enemy;
	self->enemy = NULL;

	if (self->deadflag)
	{
		self->client->buttons = 0;
		ucmd.buttons = BUTTON_ATTACK;
	}

	if(!self->deadflag)
	{
		Bot_Check_Node_Dist (self);

		if(!Bot_Find_Enemy(self))
		{
			Bot_Find_Roam_Goal(self);
			Bot_Turn_To_Goal(self);
			Bot_Move (self, &ucmd);
		}
		else
		{
			if(!ctf->value)
			{
				if(!saberonly->value)
				{
					Bot_Choose_Weapon (self);
					Bot_Find_Roam_Goal (self);
				}
				Bot_Attack (self, self->enemy, &ucmd);
			}
			else
			{
				Bot_CTF_Think (self);
				Bot_Choose_Weapon (self);
				Bot_Attack (self, self->enemy, &ucmd);
			}
		}
		if(!self->groundentity)
		{
			self->client->buttons = 0;
			ucmd.buttons = BUTTON_ATTACK;
		}
	}

	ucmd.angles[PITCH] = ANGLE2SHORT(self->s.angles[PITCH]);
	ucmd.angles[YAW] = ANGLE2SHORT(self->s.angles[YAW]);
	ucmd.angles[ROLL] = ANGLE2SHORT(self->s.angles[ROLL]);

	ucmd.msec = 50 + ceil(random()*25);
	self->client->ping = ucmd.msec;

	ClientThink (self, &ucmd);
	
	self->nextthink = level.time + FRAMETIME;
}
