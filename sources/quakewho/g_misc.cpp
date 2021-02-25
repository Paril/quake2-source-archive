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

// g_misc.c
#include "g_local.h"

#include <ctime>

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

static void Use_Areaportal (edict_t &ent, edict_t &other, edict_t &activator)
{
	ent.count ^= 1;		// toggle state
	gi.SetAreaPortalState (ent.style, ent.count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t &ent)
{
	ent.use = Use_Areaportal;
	ent.count = 0;		// always start closed;
}

//=====================================================
void BecomeExplosion1 (edict_t &self)
{
	gi.WriteByte (SVC_TEMP_ENTITY);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self.s.origin);
	gi.multicast (self.s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}

/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
	this path_corner targeted touches it
*/
void SP_path_corner (edict_t &self)
{
	if (!self.targetname)
	{
		gi.dprintf ("path_corner with no targetname at %s\n", vtos(self.s.origin));
		G_FreeEdict (self);
		return;
	}

	self.svflags |= SVF_NOCLIENT;
	self.Link();
}

/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t &self)
{
	self.absmin = self.s.origin;
	self.absmax = self.s.origin;
}

/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
				it will then blink in to existance; it will
				kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
				this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
				the wall will initially be present
*/
static void func_wall_use (edict_t &self, edict_t &other, edict_t &activator)
{
	if (self.solid == SOLID_NOT)
	{
		self.solid = SOLID_BSP;
		self.svflags &= ~SVF_NOCLIENT;
		KillBox (self);
	}
	else
	{
		self.solid = SOLID_NOT;
		self.svflags |= SVF_NOCLIENT;
	}

	self.Link();

	if (!(self.spawnflags & 2))
		self.use = nullptr;
}

void SP_func_wall (edict_t &self)
{
	self.movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self.model);

	if (self.spawnflags & 8)
		self.s.effects |= EF_ANIM_ALL;
	if (self.spawnflags & 16)
		self.s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self.spawnflags & 7) == 0)
	{
		self.solid = SOLID_BSP;
		self.Link();
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self.spawnflags & 1))
		self.spawnflags |= 1;

	// yell if the spawnflags are odd
	if ((self.spawnflags & 4) && !(self.spawnflags & 2))
	{
		gi.dprintf("func_wall START_ON without TOGGLE\n");
		self.spawnflags |= 2;
	}

	self.use = func_wall_use;

	if (self.spawnflags & 4)
		self.solid = SOLID_BSP;
	else
	{
		self.solid = SOLID_NOT;
		self.svflags |= SVF_NOCLIENT;
	}

	self.Link();
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/
static void func_object_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0f)
		return;
	if (other.takedamage == false)
		return;
	T_Damage (other, self, self, vec3_origin, self.s.origin, vec3_origin, self.dmg, 1, DAMAGE_NONE);
}

static void func_object_release (edict_t &self)
{
	self.movetype = MOVETYPE_TOSS;
	self.touch = func_object_touch;
}

static void func_object_use (edict_t &self, edict_t &other, edict_t &activator)
{
	self.solid = SOLID_BSP;
	self.svflags &= ~SVF_NOCLIENT;
	self.use = nullptr;
	KillBox (self);
	func_object_release (self);
}

constexpr vec3_t object_expansion = { 1.f, 1.f, 1.f };

void SP_func_object (edict_t &self)
{
	gi.setmodel (self, self.model);

	self.mins += object_expansion;
	self.maxs -= object_expansion;

	if (!self.dmg)
		self.dmg = 100;

	if (self.spawnflags == 0)
	{
		self.solid = SOLID_BSP;
		self.movetype = MOVETYPE_PUSH;
		self.think = func_object_release;
		self.nextthink = level.time + 200;
	}
	else
	{
		self.solid = SOLID_NOT;
		self.movetype = MOVETYPE_PUSH;
		self.use = func_object_use;
		self.svflags |= SVF_NOCLIENT;
	}

	if (self.spawnflags & 2)
		self.s.effects |= EF_ANIM_ALL;
	if (self.spawnflags & 4)
		self.s.effects |= EF_ANIM_ALLFAST;

	self.clipmask = MASK_MONSTERSOLID;

	self.Link();
}

/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1 (edict_t &ent)
{
	ent.movetype = MOVETYPE_NONE;
	ent.solid = SOLID_BBOX;
	ent.s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2");
	ent.Link();
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2 (edict_t &ent)
{
	ent.movetype = MOVETYPE_NONE;
	ent.solid = SOLID_BBOX;
	ent.s.modelindex = gi.modelindex ("models/objects/minelite/light2/tris.md2");
	ent.Link();
}

//=====================================================

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character (edict_t &self)
{
	self.movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self.model);
	self.solid = SOLID_BSP;
	self.s.frame = 12;
	self.Link();
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/
static void target_string_use (edict_t &self, edict_t &other, edict_t &activator)
{
	const size_t l = self.message ? strlen(self.message) : 0;

	for (edict_ref e = self.teammaster; e; e = e->teamchain)
	{
		if (!e->count)
			continue;

		const int32_t n = e->count - 1;

		if (n > static_cast<int32_t>(l))
		{
			e->s.frame = 12;
			continue;
		}

		const char c = self.message ? self.message[n] : '\0';

		if (c >= '0' && c <= '9')
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void SP_target_string (edict_t &self)
{
	self.use = target_string_use;
}


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"		0 "xx"
			1 "xx:xx"
			2 "xx:xx:xx"
*/

constexpr size_t CLOCK_MESSAGE_SIZE	= 16;

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

static void func_clock_reset (edict_t &self)
{
	self.activator = nullptr;
	if (self.spawnflags & 1)
	{
		self.health = 0;
		self.wait = self.count;
	}
	else if (self.spawnflags & 2)
	{
		self.health = self.count;
		self.wait = 0;
	}
}

static void func_clock_format_countdown (edict_t &self)
{
	if (self.style == 0)
	{
		snprintf (self.message, CLOCK_MESSAGE_SIZE, "%2i", self.health);
		return;
	}

	if (self.style == 1)
	{
		snprintf(self.message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self.health / 60, self.health % 60);
		if (self.message[3] == ' ')
			self.message[3] = '0';
		return;
	}

	if (self.style == 2)
	{
		snprintf(self.message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self.health / 3600, (self.health - (self.health / 3600) * 3600) / 60, self.health % 60);
		if (self.message[3] == ' ')
			self.message[3] = '0';
		if (self.message[6] == ' ')
			self.message[6] = '0';
		return;
	}
}

static void func_clock_think (edict_t &self)
{
	if (!self.enemy)
	{
		self.enemy = G_Find (nullptr, FOFS(targetname), self.target);

		if (!self.enemy)
			return;
	}

	if (self.spawnflags & 1)
	{
		func_clock_format_countdown (self);
		self.health++;
	}
	else if (self.spawnflags & 2)
	{
		func_clock_format_countdown (self);
		self.health--;
	}
	else
	{
		time_t		gmtime;
		time(&gmtime);

		tm *ltime = localtime(&gmtime);
		snprintf (self.message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

		if (self.message[3] == ' ')
			self.message[3] = '0';
		if (self.message[6] == ' ')
			self.message[6] = '0';
	}

	self.enemy->message = self.message;
	self.enemy->use (self.enemy, self, self);

	if (((self.spawnflags & 1) && (static_cast<gtime_t>(self.health) > self.wait)) ||
		((self.spawnflags & 2) && (static_cast<gtime_t>(self.health) < self.wait)))
	{
		if (self.pathtarget)
		{
			char *savetarget = self.target;
			char *savemessage = self.message;
			self.target = self.pathtarget;
			self.message = nullptr;
			G_UseTargets (self, self.activator);
			self.target = savetarget;
			self.message = savemessage;
		}

		if (!(self.spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self.spawnflags & 4)
			return;
	}

	self.nextthink = level.time + 100;
}

static void func_clock_use (edict_t &self, edict_t &other, edict_t &activator)
{
	if (!(self.spawnflags & 8))
		self.use = nullptr;
	
	if (self.activator)
		return;
	
	self.activator = activator;
	self.think (self);
}

void SP_func_clock (edict_t &self)
{
	if (!self.target)
	{
		gi.dprintf("%s with no target at %s\n", self.classname, vtos(self.s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self.spawnflags & 2) && (!self.count))
	{
		gi.dprintf("%s with no count at %s\n", self.classname, vtos(self.s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self.spawnflags & 1) && (!self.count))
		self.count = 60 * 60;

	func_clock_reset (self);

	self.message = gi.TagMalloc<char>(CLOCK_MESSAGE_SIZE, TAG_LEVEL);

	self.think = func_clock_think;

	if (self.spawnflags & 4)
		self.use = func_clock_use;
	else
		self.nextthink = level.time + 100;
}

//=================================================================================

static void teleporter_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	edict_ref dest = G_Find (nullptr, FOFS(targetname), self.target);
	
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	other.Unlink();

	other.s.origin = dest->s.origin;
	other.s.old_origin = dest->s.origin;
	other.s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	other.velocity.Clear();

	if (other.client)
	{
		// set angles
		for (size_t i = 0; i < 3; i++)
			other.client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other.client->resp.cmd_angles[i]);

		other.client->ps.viewangles.Clear();
		other.client->v_angle.Clear();
	}

	// draw the teleport splash at source and on the player
	self.owner->s.event = EV_PLAYER_TELEPORT;
	other.s.event = EV_PLAYER_TELEPORT;

	other.s.angles.Clear();

	// kill anything at the destination
	//KillBox (other);
	EnsureGoodPosition(other);

	other.Link();
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter (edict_t &ent)
{
	if (!ent.target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	ent.s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
	ent.s.skinnum = 1;
	ent.s.effects = EF_TELEPORTER;
	ent.s.sound = gi.soundindex ("world/amb10.wav");
	ent.solid = SOLID_BBOX;

	ent.mins = { -32.f, -32.f, -24.f };
	ent.maxs = { 32.f, 32.f, -16.f };
	ent.Link();

	edict_t &trig = G_Spawn ();
	trig.touch = teleporter_touch;
	trig.solid = SOLID_TRIGGER;
	trig.target = ent.target;
	trig.owner = ent;
	trig.s.origin = ent.s.origin;
	trig.mins = { -8.f, -8.f, 8.f };
	trig.maxs = { 8.f, 8.f, 24.f };
	trig.Link();
	
}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest (edict_t &ent)
{
	ent.s.modelindex = gi.modelindex("models/objects/dmspot/tris.md2");
	ent.s.skinnum = 0;
	ent.solid = SOLID_BBOX;
	ent.mins = { -32.f, -32.f, -24.f };
	ent.maxs = { 32.f, 32.f, -16.f };
	ent.Link();
}

