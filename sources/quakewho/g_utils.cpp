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
// g_utils.c -- misc utility functions for game module

#include "g_local.h"

/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if nullptr
nullptr will be returned if the end of the list is reached.

=============
*/
edict_ref G_Find (const edict_ref &from, const ptrdiff_t &fieldofs, const char *match)
{
	for (size_t n = from ? (from->s.number + 1) : 0; n < globals.entities.num; n++)
	{
		edict_t &e = g_edicts[n];

		if (!e.inuse)
			continue;

		const char *s = *reinterpret_cast<char **>(reinterpret_cast<uint8_t *>(&e) + fieldofs);

		if (!s)
			continue;
		if (iequals(s, match))
			return e;
	}

	return nullptr;
}


/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
edict_ref findradius (const edict_ref &from, const vec3_t &org, const vec_t &rad)
{
	for (size_t n = from ? (from->s.number + 1) : 0; n < globals.entities.num; n++)
	{
		edict_t &e = g_edicts[n];

		if (!e.inuse)
			continue;
		if (e.solid == SOLID_NOT)
			continue;

		const vec3_t eorg = org - (e.s.origin + (e.mins + e.maxs) * 0.5f);
		
		if (eorg.Length() > rad)
			continue;
		
		return e;
	}

	return nullptr;
}

#include <vector>

/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if nullptr
nullptr will be returned if the end of the list is reached.

=============
*/
edict_ref G_PickTarget (const char *targetname)
{
	if (!targetname)
	{
		gi.dprintf("G_PickTarget called with nullptr targetname\n");
		return nullptr;
	}

	std::vector<edict_ref> choices;
	edict_ref ent = nullptr;

	while(1)
	{
		ent = G_Find(ent, FOFS(targetname), targetname);

		if (!ent)
			break;

		choices.push_back(ent);
	}

	if (!choices.size())
	{
		gi.dprintf("G_PickTarget: target %s not found\n", targetname);
		return nullptr;
	}

	return choices[irandom(choices.size() - 1)];
}

static void Think_Delay (edict_t &ent)
{
	G_UseTargets (ent, ent.activator);
	G_FreeEdict (ent);
}

/*
==============================
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets (edict_t &ent, edict_t &activator)
{
//
// check for a delay
//
	if (ent.delay)
	{
	// create a temp object to fire at a later time
		edict_t &t = G_Spawn();
		t.classname = "DelayedUse";
		t.nextthink = level.time + ent.delay;
		t.think = Think_Delay;
		t.activator = activator;
		t.message = ent.message;
		t.target = ent.target;
		t.killtarget = ent.killtarget;
		return;
	}
	
//
// print the message
//
	if (ent.message && !(activator.svflags & SVF_MONSTER))
	{
		activator.client->CenterPrint("%s", ent.message);
		if (ent.noise_index)
			activator.PlaySound(ent.noise_index);
		else
			activator.PlaySound(gi.soundindex ("misc/talk1.wav"));
	}

//
// kill killtargets
//
	if (ent.killtarget)
	{
		edict_ref t = nullptr;
		
		while ((t = G_Find (t, FOFS(targetname), ent.killtarget)))
		{
			G_FreeEdict (t);

			if (!ent.inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

//
// fire targets
//
	if (ent.target)
	{
		edict_ref t = nullptr;

		while ((t = G_Find (t, FOFS(targetname), ent.target)))
		{
			// doors fire area portals in a specific way
			if (iequals(t->classname, "func_areaportal") &&
				(iequals(ent.classname, "func_door") || iequals(ent.classname, "func_door_rotating")))
				continue;

			if (t == ent)
				gi.dprintf ("WARNING: Entity used itself.\n");
			else if (t->use)
				t->use (t, ent, activator);

			if (!ent.inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}

#include <charconv>

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char *vtos (const vec3_t &v)
{
	static	int32_t		index;
	static	char	str[8][32];

	// use an array so that multiple vtos won't collide
	char *s = str[index];
	index = (index + 1) % lengthof(str);

	snprintf (s, 32, "(%i %i %i)", static_cast<int32_t>(v[0]), static_cast<int32_t>(v[1]), static_cast<int32_t>(v[2]));

	return s;
}

constexpr vec3_t VEC_UP			{ 0, -1, 0 };
constexpr vec3_t MOVEDIR_UP		{ 0, 0, 1 };
constexpr vec3_t VEC_DOWN		{ 0, -2, 0 };
constexpr vec3_t MOVEDIR_DOWN	{ 0, 0, -1 };

void G_SetMovedir (vec3_t &angles, vec3_t &movedir)
{
	if (angles == VEC_UP)
		movedir = MOVEDIR_UP;
	else if (angles == VEC_DOWN)
		movedir = MOVEDIR_DOWN;
	else
		movedir = angles.Forward();

	angles.Clear();
}

edict_t &G_InitEdict (edict_t &e)
{
	e.Reset();
	e.inuse = true;
	return e;
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t &G_Spawn ()
{
	// the first couple seconds of server time can involve a lot of
	// freeing and allocating, so relax the replacement policy
	for (auto &e : game.entities.range(game.clients.size() + 1))
		if (!e.inuse && (e.freetime < 2000 || level.time - e.freetime > 500))
			return G_InitEdict(e);
	
	if (globals.entities.num == globals.entities.max)
		gi.error ("ED_Alloc: no free edicts");

	return G_InitEdict(g_edicts[globals.entities.num++]);
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict (edict_t &ed)
{
	if (ed.s.number != &ed - g_edicts)
		gi.error("entity broken\n");

	ed.Unlink();

	if (static_cast<size_t>(ed.s.number) <= game.clients.size())
	{
		gi.dprintf("tried to free special edict\n");
		return;
	}

	ed.Reset();
	ed.classname = "freed";
	ed.freetime = level.time;
}


/*
============
G_TouchTriggers

============
*/
void G_TouchTriggers (edict_t &ent)
{
	// dead things don't activate triggers!
	if ((ent.client || (ent.svflags & SVF_MONSTER)) && (ent.health <= 0))
		return;

	edict_ref touch[MAX_EDICTS];
	const size_t num = gi.BoxEdicts (ent.absmin, ent.absmax, touch, MAX_EDICTS, AREA_TRIGGERS);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (size_t i = 0; i < num; i++)
	{
		edict_ref &hit = touch[i];

		if (hit->inuse && hit->touch)
			hit->touch (hit, ent, nullptr, nullptr);
	}
}

/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
bool KillBox (edict_t &ent)
{
	while (1)
	{
		trace_t tr = gi.trace (ent.s.origin, ent.mins, ent.maxs, ent.s.origin, nullptr, MASK_PLAYERSOLID);
		
		if (tr.ent != game.world())
			break;

		// nail it
		T_Damage (tr.ent, ent, ent, vec3_origin, ent.s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION);

		// if we didn't kill it, fail
		if (tr.ent->solid)
			return false;
	}

	return true;		// all clear
}
