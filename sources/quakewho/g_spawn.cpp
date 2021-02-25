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

spawn_temp_t	st;

static void SP_worldspawn (edict_t &ent);

constexpr struct {
	std::string_view name;
	void (*spawn)(edict_t &ent);
} spawns[] = {
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
	{"func_killbox", SP_func_killbox},

	{"trigger_always", SP_trigger_always},
	{"trigger_once", SP_trigger_once},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_relay", SP_trigger_relay},
	{"trigger_push", SP_trigger_push},
	{"trigger_hurt", SP_trigger_hurt},
	{"trigger_counter", SP_trigger_counter},
	{"trigger_elevator", SP_trigger_elevator},
	{"trigger_gravity", SP_trigger_gravity},
	{"trigger_monsterjump", SP_trigger_monsterjump},

	{"target_temp_entity", SP_target_temp_entity},
	{"target_speaker", SP_target_speaker},
	{"target_explosion", SP_target_explosion},
	{"target_splash", SP_target_splash},
	{"target_blaster", SP_target_blaster},
	{"target_laser", SP_target_laser},
	{"target_earthquake", SP_target_earthquake},
	{"target_character", SP_target_character},
	{"target_string", SP_target_string},

	{"worldspawn", SP_worldspawn},

	{"light_mine1", SP_light_mine1},
	{"light_mine2", SP_light_mine2},
	{"info_notnull", SP_info_notnull},
	{"path_corner", SP_path_corner},
	{"misc_teleporter", SP_misc_teleporter},
	{"misc_teleporter_dest", SP_misc_teleporter_dest}
};

/*
===============
ED_CallSpawn

Finds the spawn function for the entity and calls it
===============
*/
void ED_CallSpawn (edict_t &ent)
{
	if (!ent.classname)
	{
		gi.dprintf ("ED_CallSpawn: nullptr classname\n");
		return;
	}

	// check normal spawn functions
	for (auto &s : spawns)
	{
		if (!iequals(s.name, ent.classname))
			continue;

		// found it
		ent.classname = s.name.data();
		s.spawn (ent);
		return;
	}

#if defined(DEBUG)
	gi.dprintf ("%s doesn't have a spawn function\n", ent.classname);
#endif

	G_FreeEdict(ent);
}

/*
=============
ED_NewString
=============
*/
static char *ED_NewString (const std::string_view &string)
{	
	const size_t l = string.size();
	char *newb = gi.TagMalloc<char>(l + 1, TAG_LEVEL);
	char *new_p = newb;

	for (size_t i = 0; i < l; i++)
	{
		if (string[i] == '\\' && i < l - 1)
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

	*new_p = 0;
	
	return newb;
}

#include "g_fields.h"

/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
static void ED_ParseField (const std::string_view &key, const std::string_view &value, edict_t &ent)
{
	for (auto &f : fields)
	{
		if (!iequals(f.name, key))
			continue;

		// found it
		uint8_t *b;
		vec_t v;
		vec3_t vec;

		if (f.flags & FFL_SPAWNTEMP)
			b = reinterpret_cast<uint8_t *>(&st);
		else
			b = reinterpret_cast<uint8_t *>(&ent);

		char *value_str = ED_NewString(value);

		switch (f.type)
		{
		case F_LSTRING:
			*reinterpret_cast<char **>(b + f.ofs) = value_str;
			return;
		case F_VECTOR:
			sscanf (value_str, "%f %f %f", &vec[0], &vec[1], &vec[2]);
			*reinterpret_cast<vec3_t *>(b + f.ofs) = vec;
			break;
		case F_INT:
			*reinterpret_cast<int32_t *>(b + f.ofs) = strtol(value_str, nullptr, 10);
			break;
		case F_FLOAT:
			*reinterpret_cast<vec_t *>(b + f.ofs) = strtof(value_str, nullptr);
			break;
		case F_ANGLEHACK:
			v = atof(value_str);
			*reinterpret_cast<vec3_t *>(b + f.ofs) = { 0.f, v, 0.f };
			break;
		case F_TIME:
			*reinterpret_cast<gtime_t *>(b + f.ofs) = strtoull(value_str, nullptr, 10) * 1000;
			break;
		case F_IGNORE:
			break;
		}

		gi.TagFree(value_str);
		return;
	}

	gi.dprintf ("%.*s is not a field\n", key.size(), key.data());
}

/*
====================
ED_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
static void ED_ParseEdict (com_parse_t &last_token, edict_t &ent)
{
	bool init = false;

	st = spawn_temp_t();

// go through all the dictionary pairs
	while (1)
	{	
	// parse key
		if (!COM_Parse (last_token))
			gi.error ("ED_ParseEntity: EOF without closing brace");
		
		if (last_token.token.data()[0] == '}')
			break;

		const std::string_view key_token = last_token.token;

	// parse value
		if (!COM_Parse (last_token) || !last_token.token.size())
			gi.error ("ED_ParseEntity: EOF without closing brace");
		
		if (last_token.token.data()[0] == '}')
			break;

		init = true;	

	// keynames with a leading underscore are used for utility comments,
	// and are immediately discarded by quake
		if (key_token.data()[0] == '_')
			continue;

		ED_ParseField (key_token, last_token.token, ent);
	}
}


/*
================
G_FindTeams

Chain together all entities with a matching team field.

All but the first will have the FL_TEAMSLAVE flag set.
All but the last will have the teamchain field set to the next one
================
*/
static void G_FindTeams ()
{
	int32_t c = 0, c2 = 0;

	for (auto &e : game.entities)
	{
		if (!e.inuse)
			continue;
		if (!e.team)
			continue;
		if (e.flags & FL_TEAMSLAVE)
			continue;

		edict_ref chain = e;
		e.teammaster = e;
		c++;
		c2++;

		for (auto &e2 : game.entities.range(e.s.number + 1))
		{
			if (!e2.inuse)
				continue;
			if (!e2.team)
				continue;
			if (e2.flags & FL_TEAMSLAVE)
				continue;

			if (!strcmp(e.team, e2.team))
			{
				c2++;
				chain->teamchain = e2;
				e2.teammaster = e;
				chain = e2;
				e2.flags |= FL_TEAMSLAVE;
			}
		}
	}

	gi.dprintf ("%i teams with %i entities\n", c, c2);
}

std::unordered_map<grid_array, nav_grid_node> grid;

constexpr vec_t grid_to_vec(const uint8_t &val)
{
	return (val * spawn_grid_size) - 4096.f;
}

constexpr uint8_t vec_to_grid(const vec_t &vec)
{
	return static_cast<uint8_t>((vec + 4096) / spawn_grid_size);
}

constexpr vec3_t grid_to_vec_array(const grid_array &val)
{
	return { grid_to_vec(val[0]), grid_to_vec(val[1]), grid_to_vec(val[2]) };
}

constexpr grid_array vec_to_grid_array(const vec3_t &val)
{
	return { vec_to_grid(val[0]), vec_to_grid(val[1]), vec_to_grid(val[2]) };
}

constexpr vec3_t small_monster_mins = { -16, -16, 0 };
constexpr vec3_t small_monster_maxs = { 16, 16, 56 };

static brushcontents_t PointContents(vec3_t pos)
{
	brushcontents_t contents = gi.pointcontents(pos);

	if (contents & (CONTENTS_PLAYERCLIP | CONTENTS_MONSTERCLIP))
		return gi.trace(pos, pos, MASK_ALL & ~(CONTENTS_DEADMONSTER | CONTENTS_MONSTER)).contents;

	return contents;
}

static bool CheckGoodOffsettedPoint(vec3_t &point, trace_t &tr)
{
	static int8_t offsets[] { 0, 8, 16, -8, -16 };

	for (int8_t offset : offsets)
	{
		vec3_t offsetted { point[0], point[1], point[2] + offset };

		brushcontents_t c = PointContents(offsetted) & ~CONTENTS_WATER;

		if (c == CONTENTS_NONE)
		{
			tr = gi.trace(offsetted, vec3_t { offsetted[0], offsetted[1], offsetted[2] - 1024 }, MASK_CLIP);
			point = offsetted;
			return true;
		}
	}

	return false;
}

static void AttemptPositioningFix(const vec3_t dir, vec3_t end)
{
	vec3_t negative, positive;

	negative = end + (dir * -16);
	positive = end + (dir * 16);

	// no fit; let's shift the x/y around. first, check X.
	trace_t x_trace = gi.trace(negative, positive, MASK_CLIP);

	// -16 to 16 is inside a wall *or* in a good spot already, try the opposite
	if (x_trace.startsolid || x_trace.allsolid || x_trace.fraction == 1.0f)
		x_trace = gi.trace(positive, negative, MASK_CLIP);
		
	// we hit a wall; project outwards from this wall piece
	if (!x_trace.startsolid && !x_trace.allsolid && x_trace.fraction < 1.0f)
		end = x_trace.endpos + (x_trace.plane.normal * 16);
}

static bool PointIsGood(vec3_t &point)
{
	trace_t tr;

	if (!CheckGoodOffsettedPoint(point, tr))
		return false;

	vec3_t end { tr.endpos[0], tr.endpos[1], tr.endpos[2] + 0.125f };

	if (/*tr.startsolid || */tr.allsolid || tr.fraction >= 1.0f || tr.plane.normal[2] < 0.7f)
		return false;

	if ((tr.surface->flags & (SURF_SKY | SURF_NODRAW)) || PointContents(end) != CONTENTS_NONE)
		return false;

	end += tr.plane.normal * 16.f;

	// test monster positioning
	trace_t fit_trace = gi.trace(end, small_monster_mins, small_monster_maxs, end, MASK_CLIP);

	if (fit_trace.startsolid || fit_trace.allsolid)
	{
		AttemptPositioningFix(vec3_t { 1, 0, 0 }, end);

		fit_trace = gi.trace(end, small_monster_mins, small_monster_maxs, end, MASK_CLIP);

		if (fit_trace.startsolid || fit_trace.allsolid)
		{
			AttemptPositioningFix(vec3_t { 0, 1, 0 }, end);

			fit_trace = gi.trace(end, small_monster_mins, small_monster_maxs, end, MASK_CLIP);
		}
	}

	if (fit_trace.startsolid || fit_trace.allsolid)
		return false;

	for (int16_t z = vec_to_grid(point[2]); z >= 0; z--)
	{
		const grid_array g { vec_to_grid(point[0]), vec_to_grid(point[1]), static_cast<uint8_t>(z) };
		
		if (!grid.contains(g))
			continue;

		auto &p = grid[g];

		if (p.position.Distance(end) < 1)
			return false;
	}

	point = end;
	return true;
}

std::vector<std::pair<grid_array, nav_grid_node*>> nav_points;

static monster_def_t monster_funcs[] = {
	{ SP_monster_berserk, "Berserker" },
	{ SP_monster_gladiator, "Gladiator" },
	{ SP_monster_gunner, "Gunner" },
	{ SP_monster_infantry, "Infantry" },
	{ SP_monster_soldier_light, "Light Soldier" },
	{ SP_monster_soldier, "Shotgun Soldier" },
	{ SP_monster_soldier_ss, "Machinegun Soldier" },
	{ SP_monster_tank, "Tank" },
	{ SP_monster_medic, "Medic" },
	{ SP_monster_chick, "Iron Maiden" },
	{ SP_monster_parasite, "Parasite" },
	{ SP_monster_brain, "Brain" },
	{ SP_monster_mutant, "Mutant" }
};

monster_def_t &G_RandomMonster()
{
	return monster_funcs[irandom(lengthof(monster_funcs) - 1)];
}

template<typename T>
struct std::hash<std::tuple<T, T>>
{
	size_t operator()(const std::tuple<T, T>& _Keyval) const
	{
		std::hash<T> ga;
		return ga(std::get<0>(_Keyval)) ^ ga(std::get<1>(_Keyval));
	}
};

static nav_grid_node *ClosestNode(const vec3_t position)
{
	nav_grid_node *best = nullptr;
	float best_dist = -1;

	for (auto &pt : grid)
	{
		float len = position.Distance(pt.second.position);

		if (!best || len < best_dist)
		{
			best = &pt.second;
			best_dist = len;
		}
	}

	return best;
}

#ifdef ASTAR
using AStar_EstimateCost = float (*)(nav_grid_node *node, nav_grid_node *goal);

struct nav_grid_astar
{
	nav_grid_node *node;
	float fScore;
};

template<>
struct std::less<nav_grid_astar>
{
	constexpr bool operator()(const nav_grid_astar &lhs, const nav_grid_astar &rhs) const 
	{
		return lhs.fScore < rhs.fScore;
	}
};

static std::vector<nav_grid_node *> AStarReconstruct(std::unordered_map<nav_grid_node *, nav_grid_node *> &cameFrom, nav_grid_node *current)
{
    std::vector<nav_grid_node *> total_path { current };

    while (cameFrom.contains(current))
	{
        current = cameFrom[current];
        total_path.push_back(current);
	}
	
	std::reverse(total_path.begin(), total_path.end());
    return total_path;
}

static std::vector<nav_grid_node *> AStar(nav_grid_node *start, nav_grid_node *goal, AStar_EstimateCost h, AStar_EstimateCost d)
{
	std::set<nav_grid_astar> openSet;
	openSet.emplace(nav_grid_astar { .node = start, .fScore = h(start, goal) });

	std::unordered_set<nav_grid_node*> openSetNodes;
	openSetNodes.emplace(start);

	std::unordered_map<nav_grid_node *, nav_grid_node *> cameFrom;
	
	std::unordered_map<nav_grid_node *, float> gScore;
	gScore[start] = 0;

	while (!openSet.empty())
	{
		auto current = openSet.begin();
		auto currentNode = (*current).node;

        if (currentNode == goal)
            return AStarReconstruct(cameFrom, (*current).node);

		openSet.erase(current);
		openSetNodes.erase(currentNode);

        for (auto &neighbor_id : currentNode->connections)
		{
			auto neighbor = &grid[neighbor_id];
            auto tentative_gScore = gScore[currentNode] + d(currentNode, neighbor);

            if (tentative_gScore < (gScore.contains(neighbor) ? gScore[neighbor] : INFINITY))
			{
                cameFrom[neighbor] = currentNode;
                gScore[neighbor] = tentative_gScore;

				float score = gScore[neighbor] + h(neighbor, goal);

                if (!openSetNodes.contains(neighbor))
				{
					openSet.emplace(nav_grid_astar { .node = neighbor, .fScore = score });
					openSetNodes.emplace(neighbor);
				}
				else for (auto it = openSet.begin(); it != openSet.end(); it++)
				{
					if ((*it).node != neighbor)
						continue;

					openSet.erase(it);
					openSet.emplace(nav_grid_astar { .node = neighbor, .fScore = score });
					break;
				}
			}
		}
	}

	return std::vector<nav_grid_node *>();
}

static std::vector<nav_grid_node*> the_path;
static std::vector<nav_grid_node*>::iterator path_iterator;

static float Estimate(nav_grid_node *current, nav_grid_node *goal)
{
	return current->position.Distance(goal->position);
}
#endif

void Cmd_Pos_f(const edict_t &ent)
{
	return;
	/*auto v = grid_to_vec_array(vec_to_grid_array({ ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] }));
	PointIsGood(v);
	gi.WriteByte(SVC_TEMP_ENTITY);
	gi.WriteByte(TE_FLECHETTE);
	gi.WritePosition(v.data());
	gi.WriteDir(vec3_t { 0, 0, 1 });
	gi.multicast(v.data(), MULTICAST_ALL);
	return;*/

	/*nav_grid_node *goal = ClosestNode(G_Find(nullptr, FOFS(classname), "info_player_deathmatch")->s.origin);
	nav_grid_node *current = ClosestNode(ent.s.origin);

	the_path = AStar(current, goal, Estimate, Estimate);
	path_iterator = the_path.begin();

	ent.client->Print("path: %u len\n", the_path.size());*/
}

void DrawPoints()
{
#ifdef NODRAW
	int draw_count = 0;

	nav_grid_node *prev = nullptr;

	if (the_path.size())
	for (; ; path_iterator++)
	{
		if (path_iterator == the_path.end())
		{
			path_iterator = the_path.begin();
			prev = nullptr;
		}
		
		auto cur = *path_iterator;

		if (prev)
		{
			gi.WriteByte(SVC_TEMP_ENTITY);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(prev->position.data());
			gi.WritePosition(cur->position.data());
			gi.multicast(cur->position.data(), MULTICAST_ALL);

			if (++draw_count > 8)
				break;
		}

		prev = cur;
	}

	auto closest = ClosestNode(g_edicts[1].s.origin);

	if (closest)
	{
		/*gi.WriteByte(SVC_TEMP_ENTITY);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(vec3_t { closest->position[0] - 16, closest->position[1], closest->position[2] });
		gi.WritePosition(vec3_t { closest->position[0] + 16, closest->position[1], closest->position[2] });
		gi.multicast(closest->position.data(), MULTICAST_ALL);
		
		gi.WriteByte(SVC_TEMP_ENTITY);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(vec3_t { closest->position[0], closest->position[1] - 16, closest->position[2] });
		gi.WritePosition(vec3_t { closest->position[0], closest->position[1] + 16, closest->position[2] });
		gi.multicast(closest->position.data(), MULTICAST_ALL);*/

		for (auto &connection : closest->connections)
		{
			auto connected_node = &grid[connection];

			gi.WriteByte(SVC_TEMP_ENTITY);
			gi.WriteByte(TE_BFG_LASER);
			gi.WritePosition(closest->position.data());
			gi.WritePosition(connected_node->position.data());
			gi.multicast(closest->position.data(), MULTICAST_ALL);
		}
	}
#endif
}

static void PrecacheMonsters()
{
	edict_t &ent = G_Spawn();

	for (auto func : monster_funcs)
		func.func(ent);

	G_FreeEdict(ent);
}

static void FindGridConnections(std::pair<const grid_array, nav_grid_node> &node)
{
	for (int8_t x = -1; x <= 1; x++)
	for (int8_t y = -1; y <= 1; y++)
	for (int8_t z = 0; z <= node.first[2] + 1; z++)
	{
		if (x == 0 && y == 0 && z == node.first[2])
			continue;
		if (node.first[0] + x < 0 || node.first[1] + y < 0 ||
			node.first[0] + x >= static_cast<int32_t>(grid_max) || node.first[1] + y >= static_cast<int32_t>(grid_max))
			continue;

		const grid_array grid_val { static_cast<uint8_t>(node.first[0] + x), static_cast<uint8_t>(node.first[1] + y), static_cast<uint8_t>(z) };

		if (!grid.contains(grid_val))
			continue;

		const auto &n = grid[grid_val];

		trace_t tr = gi.trace(node.second.position, vec3_origin, vec3_origin, n.position, nullptr, MASK_CLIP);

		if (tr.fraction != 1.0f)
		{
			// try a step up; trace from cur to up, then up to node
			if (n.position[2] > node.second.position[2])
			{
				vec3_t step_up = node.second.position;

				while (fabs(step_up[2] - n.position.data()[2]))
				{
					// go upwards
					vec3_t moved_up = { step_up[0], step_up[1], min(n.position[2], step_up[2] + 18) };
					tr = gi.trace(step_up, vec3_origin, vec3_origin, moved_up, nullptr, MASK_CLIP);

					step_up = moved_up;

					// must be clear
					if (tr.fraction == 1.0f)
					{
						// go towards the node
						tr = gi.trace(step_up, vec3_origin, vec3_origin, n.position, nullptr, MASK_CLIP);

						// we potentially hit a stair; copy endpos, try again on next loop
						if (tr.fraction < 1.0f)
							step_up = { tr.endpos[0], tr.endpos[1], tr.endpos[2] };
					}

					if (tr.fraction == 1.0f)
						break;
				}
			}

			if (tr.fraction != 1.0f && n.position[2] < node.second.position[2])
			{
				// try a step down; trace from cur to node.xy, then node.xy to node
				tr = gi.trace(node.second.position, vec3_origin, vec3_origin, vec3_t { n.position[0], n.position[1], node.second.position[2] }, nullptr, MASK_CLIP);

				// missed straight trace, try a step up first
				if (tr.fraction != 1.0f)
				{
					trace_t step_up = gi.trace(node.second.position, vec3_origin, vec3_origin, vec3_t { node.second.position[0], node.second.position[1], node.second.position[2] + 18 }, nullptr, MASK_CLIP);

					if (step_up.fraction == 1.0f)
						tr = gi.trace(vec3_t { node.second.position[0], node.second.position[1], node.second.position[2] + 18 }, vec3_origin, vec3_origin, vec3_t { n.position[0], n.position[1], node.second.position[2] + 18 }, nullptr, MASK_CLIP);
				}

				if (tr.fraction == 1.0f)
					tr = gi.trace(vec3_t { n.position[0], n.position[1], node.second.position[2] }, vec3_origin, vec3_origin, n.position, nullptr, MASK_CLIP);
			}
		}

		if (tr.fraction != 1.0f || tr.startsolid || tr.allsolid)
			continue;
	
		node.second.connections.emplace(grid_val);
	}
}

static void FloodFillMark(nav_grid_node *node, std::unordered_set<grid_array> &visited_nodes)
{
	if (!node || visited_nodes.contains(node->grid_position))
		return;

	visited_nodes.emplace(node->grid_position);

	for (auto connection : node->connections)
		FloodFillMark(&grid[connection], visited_nodes);
}

static void FindSpawnPoints()
{
	std::unordered_set<vec3_t> hashed_points;

	for (uint8_t z = 0; z < grid_max; z++)
	for (uint8_t y = 0; y < grid_max; y++)
	for (uint8_t x = 0; x < grid_max; x++)
	{
		const grid_array grid_val { x, y, z };
		vec3_t point = grid_to_vec_array(grid_val);

		if (!PointIsGood(point))
			continue;

		hashed_points.emplace(point);
		grid[grid_val] = nav_grid_node { .grid_position = grid_val, .position = point };
	}

	for (auto &p : grid)
		FindGridConnections(p);

	std::unordered_set<grid_array> visited_nodes;
	edict_ref point = nullptr;

	while ((point = G_Find(point, FOFS(classname), "info_player_deathmatch")))
		FloodFillMark(ClosestNode(point->s.origin), visited_nodes);
	
	for (auto it = grid.begin(); it != grid.end(); )
	{
		if (!visited_nodes.contains((*it).first))
			it = grid.erase(it);
		else
			it++;
	}

	for (auto &p : grid)
		nav_points.push_back(std::make_pair(p.first, &p.second));

	std::sort(nav_points.begin(), nav_points.end(), [] (auto &x, auto &y) {
		if (x.second->position[2] == y.second->position[2])
		{
			if (x.second->position[1] == y.second->position[1])
				return x.second->position[0] > y.second->position[0];

			return x.second->position[1] > y.second->position[1];
		}

		return x.second->position[2] > y.second->position[2];
	});

	level.max_monsters = nav_points.size() / 8;
	level.end_max_monsters = level.max_monsters / 4;

	if (roundlimit->value > 60)
		level.monster_kill_time = ((roundlimit->value - 60) / (level.max_monsters - level.end_max_monsters)) * 1000;
	else
		level.monster_kill_time = 0;

	gi.dprintf("Found %u spawn points; %u monsters will spawn, down to %u, -1 every %u sec\n", grid.size(), level.max_monsters, level.end_max_monsters, static_cast<uint32_t>(level.monster_kill_time / 1000));
}

/*
==============
SpawnEntities

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
==============
*/
void SpawnEntities (const char *mapname, const char *entities, const char *spawnpoint)
{
	const cvar_t *deathmatch = gi.cvar ("deathmatch", "1", CVAR_LATCH);
	const cvar_t *coop = gi.cvar ("coop", "0", CVAR_LATCH);

	if (!deathmatch->value || coop->value)
	{
		gi.dprintf("Needs dm 1/coop 0, setting...\n");
		gi.cvar_forceset("sv_allow_map", "1");
		gi.cvar_forceset("deathmatch", "1");
		gi.cvar_forceset("coop", "0");
		gi.AddCommandString(va("map %s\n", mapname));
	}

	SaveClientData ();

	gi.FreeTags (TAG_LEVEL);

	level = level_locals_t();

	strncpy (level.mapname, mapname, sizeof(level.mapname)-1);

	// set client fields on player ents
	for (auto &e : game.entities.range(0, globals.entities.max))
		e.Reset();

	for (auto &player : game.players)
		player.client = &game.clients[player.s.number - 1];

	edict_ref ent = game.world();
	size_t inhibit = 0;
	com_parse_t parse = { .start = entities };

// parse ents
	while (1)
	{
		// parse the opening brace	
		if (!COM_Parse (parse))
			break;
		
		// if our last entity spawned, prep a new entity ID 
		if (ent)
			ent = G_Spawn();
		else if (!ent->inuse)
			G_InitEdict(ent);

		if (parse.token.data()[0] != '{')
			gi.error ("%s: found %.*s when expecting {", __FUNCTION__, parse.token.size(), parse.token.data());

		ED_ParseEdict (parse, ent);

		// remove things (except the world) from different skill levels or deathmatch
		if (ent->s.number)
		{
			if (ent->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
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

	FindSpawnPoints();

	G_FindTeams ();
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
	endif

#endif

#include <sstream>

constexpr int16_t stat(const statindex_t &stat) { return static_cast<int16_t>(stat); }

static const std::string dm_statusbar = (std::stringstream() <<
	"yb	-24 " <<

	// health
	"if " << stat(STAT_HUNTER) << " xv 0 hnum xv 50 pic " << stat(STAT_HEALTH_ICON) << " endif " <<
	"if " << stat(STAT_HIDER) << " xv 0 hnum xv 50 pic " << stat(STAT_HEALTH_ICON) << " endif " <<

	// ammo
	"if " << stat(STAT_AMMO_ICON) << " " <<
	 "xv 100 " <<
	 "anum " <<
	 "xv 150 " <<
	 "pic " << stat(STAT_AMMO_ICON) << " " <<
	"endif " <<

	//  frags
	"xl	0 " <<
	"yt 2 " <<
	"num 3 " << stat(STAT_FRAGS) << " " <<

	// spectator
	"if " << stat(STAT_SPECTATOR) << " " <<
	 "xv 0 " <<
	 "yb -58 " <<
	 "string2 \"SPECTATOR MODE; PRESS 1 TO READY UP\" " <<
	"endif " <<

	// control notice
	"if " << stat(STAT_CONTROL) << " " <<
	 "yb -92 " <<
	 "xv 0 " <<
	 "stat_string " << stat(STAT_CONTROL) << " " <<
	"endif " <<

	// radar notice
	"if " << stat(STAT_RADAR) << " " <<
	 "yb -82 " <<
	 "xv 0 " <<
	 "stat_string " << stat(STAT_RADAR) << " " <<
	"endif " <<

	// chase camera
	"if " << stat(STAT_CHASE) << " " <<
	 "xv 0 " <<
	 "yb -68 " <<
	 "string \"Chasing\" " <<
	 "xv 64 " <<
	 "stat_string " << stat(STAT_CHASE) << " " <<
	"endif " <<

	// ammo types
	"if " << stat(STAT_HUNTER) << " " <<
	 "xr -28 " <<
	 
	// "yt 64 " <<
	// "picn w_machinegun " <<
	 
	 "yt 92 " <<
	 "picn w_shotgun " <<
	 
	 "yt 120 " <<
	 "picn w_glauncher " <<
	 
	 "xr -80 " <<
	 
	// "yt 64 " <<
	// "num 3 " << STAT_BULLETS << " " <<
	 
	 "yt 92 " <<
	 "num 3 " << stat(STAT_SHELLS) << " " <<
	 
	 "yt 120 " <<
	 "num 3 " << stat(STAT_GRENADES) << " " <<
	"endif " <<

	"if " << stat(STAT_ROUND_TIMER) << " " <<
	 "xr -138 " <<
	 "yt 32 " <<
	 "stat_string " << stat(STAT_ROUND_TIMER) << " " <<
	"endif "
	).str();;

modelindex_t	sm_meat_index;
soundindex_t	snd_fry;

/*QUAKED worldspawn (0 0 0) ?

Only used for the world.
"sky"	environment map name
"skyaxis"	vector axis for rotating sky
"skyrotate"	speed of rotation in degrees/second
"sounds"	music cd track number
"gravity"	800 is default gravity
"message"	text to print at user logon
*/
static void SP_worldspawn (edict_t &ent)
{
	ent.movetype = MOVETYPE_PUSH;
	ent.solid = SOLID_BSP;
	ent.inuse = true;			// since the world doesn't use G_Spawn()
	ent.s.modelindex = MODEL_WORLD;		// world model is always index 1

	//---------------

	if (st.nextmap)
		strcpy (level.nextmap, st.nextmap);

	// make some data visible to the server

	if (ent.message && ent.message[0])
	{
		gi.configstring (CS_NAME, ent.message);
		strncpy (level.level_name, ent.message, sizeof(level.level_name));
	}
	else
		strncpy (level.level_name, level.mapname, sizeof(level.level_name));

	if (st.sky && st.sky[0])
		gi.configstring (CS_SKY, st.sky);
	else
		gi.configstring (CS_SKY, "unit1_");

	gi.configstring (CS_SKYROTATE, va("%f", static_cast<double>(st.skyrotate)));

	gi.configstring (CS_SKYAXIS, va("%f %f %f", static_cast<double>(st.skyaxis[0]), static_cast<double>(st.skyaxis[1]), static_cast<double>(st.skyaxis[2])));

	gi.configstring (CS_CDTRACK, va("%i", ent.sounds));

	gi.configstring (CS_MAXCLIENTS, va("%u", game.clients.size()));

	// status bar program
	gi.configstring (CS_STATUSBAR, dm_statusbar.c_str());

	// items
	InitItems ();

	//---------------

	level.pic_health = gi.imageindex ("i_health");

	if (!st.gravity)
		gi.cvar_set("sv_gravity", "800");
	else
		gi.cvar_set("sv_gravity", st.gravity);

	snd_fry = gi.soundindex ("player/fry.wav");	// standing in lava / slime

	gi.soundindex ("player/lava1.wav");
	gi.soundindex ("player/lava2.wav");

	gi.soundindex ("misc/talk1.wav");

	gi.soundindex ("misc/udeath.wav");

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

	gi.soundindex ("weapons/noammo.wav");

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

	PrecacheMonsters();
}

