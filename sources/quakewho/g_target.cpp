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

/*QUAKED target_temp_entity (1 0 0) (-8 -8 -8) (8 8 8)
Fire an origin based temp entity event to the clients.
"style"		type uint8_t
*/
static void Use_Target_Tent (edict_t &ent, edict_t &other, edict_t &activator)
{
	gi.WriteByte (SVC_TEMP_ENTITY);
	gi.WriteByte (ent.style);
	gi.WritePosition (ent.s.origin);
	gi.multicast (ent.s.origin, MULTICAST_PVS);
}

void SP_target_temp_entity (edict_t &ent)
{
	ent.use = Use_Target_Tent;
}

//==========================================================

//==========================================================

/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off reliable
"noise"		wav file to play
"attenuation"
-1 = none, send to whole level
1 = normal fighting sounds
2 = idle sound level
3 = ambient sound level
"volume"	0.0 to 1.0

Normal sounds play each time the target is used.  The reliable flag can be set for crucial voiceovers.

Looped sounds are always atten 3 / vol 1, and the use function toggles it on/off.
Multiple identical looping sounds will just increase volume without any speed cost.
*/
static void Use_Target_Speaker (edict_t &ent, edict_t &other, edict_t &activator)
{
	if (ent.spawnflags & 3)
	{
		// looping sound toggles
		if (ent.s.sound)
			ent.s.sound = SOUND_NONE;	// turn it off
		else
			ent.s.sound = ent.noise_index;	// start it
	}
	else
	{
		soundchannel_t chan;
		
		// normal sound
		if (ent.spawnflags & 4)
			chan = CHAN_VOICE | CHAN_RELIABLE;
		else
			chan = CHAN_VOICE;

		// use a positioned_sound, because this entity won't normally be
		// sent to any clients because it is invisible
		ent.PlayPositionedSound(ent.noise_index, ent.s.origin, chan, static_cast<soundattn_t>(ent.attenuation), ent.volume);
	}
}

void SP_target_speaker (edict_t &ent)
{
	if (!st.noise)
	{
		gi.dprintf("target_speaker with no noise set at %s\n", vtos(ent.s.origin));
		return;
	}

	const char *buffer = st.noise;
	
	if (!strstr(buffer, ".wav"))
		buffer = va("%s.wav", buffer);
	
	ent.noise_index = gi.soundindex(buffer);

	if (!ent.volume)
		ent.volume = 1.0;

	if (!ent.attenuation)
		ent.attenuation = 1.0;
	else if (ent.attenuation == -1)	// use -1 so 0 defaults to 1
		ent.attenuation = 0;

	// check for prestarted looping sound
	if (ent.spawnflags & 1)
		ent.s.sound = ent.noise_index;

	ent.use = Use_Target_Speaker;

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	ent.Link();
}

//==========================================================


/*QUAKED target_explosion (1 0 0) (-8 -8 -8) (8 8 8)
Spawns an explosion temporary entity when used.

"delay"		wait this long before going off
"dmg"		how much radius damage should be done, defaults to 0
*/
static void target_explosion_explode (edict_t &self)
{
	gi.WriteByte (SVC_TEMP_ENTITY);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self.s.origin);
	gi.multicast (self.s.origin, MULTICAST_PHS);

	T_RadiusDamage (self, self.activator, self.dmg, nullptr, self.dmg + 40);

	const vec_t save = self.delay;
	self.delay = 0;
	G_UseTargets (self, self.activator);
	self.delay = save;
}

static void use_target_explosion (edict_t &self, edict_t &other, edict_t &activator)
{
	self.activator = activator;

	if (!self.delay)
	{
		target_explosion_explode (self);
		return;
	}

	self.think = target_explosion_explode;
	self.nextthink = level.time + self.delay;
}

void SP_target_explosion (edict_t &ent)
{
	ent.use = use_target_explosion;
	ent.svflags = SVF_NOCLIENT;
}

//==========================================================

/*QUAKED target_splash (1 0 0) (-8 -8 -8) (8 8 8)
Creates a particle splash effect when used.

Set "sounds" to one of the following:
  1) sparks
  2) blue water
  3) brown water
  4) slime
  5) lava
  6) blood

"count"	how many pixels in the splash
"dmg"	if set, does a radius damage at this location when it splashes
		useful for lava/sparks
*/
static void use_target_splash (edict_t &self, edict_t &other, edict_t &activator)
{
	gi.WriteByte (SVC_TEMP_ENTITY);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (self.count);
	gi.WritePosition (self.s.origin);
	gi.WriteDir (self.movedir);
	gi.WriteByte (self.sounds);
	gi.multicast (self.s.origin, MULTICAST_PVS);

	if (self.dmg)
		T_RadiusDamage (self, activator, self.dmg, nullptr, self.dmg + 40);
}

void SP_target_splash (edict_t &self)
{
	self.use = use_target_splash;
	G_SetMovedir (self.s.angles, self.movedir);

	if (!self.count)
		self.count = 32;

	self.svflags = SVF_NOCLIENT;
}

//==========================================================

/*QUAKED target_blaster (1 0 0) (-8 -8 -8) (8 8 8) NOTRAIL NOEFFECTS
Fires a blaster bolt in the set direction when triggered.

dmg		default is 15
speed	default is 1000
*/
static void use_target_blaster (edict_t &self, edict_t &other, edict_t &activator)
{
	entity_effects_t effect;

	if (self.spawnflags & 2)
		effect = EF_NONE;
	else if (self.spawnflags & 1)
		effect = EF_HYPERBLASTER;
	else
		effect = EF_BLASTER;

	fire_blaster (self, self.s.origin, self.movedir, self.dmg, self.speed, effect);
	self.PlaySound(self.noise_index, CHAN_VOICE);
}

void SP_target_blaster (edict_t &self)
{
	self.use = use_target_blaster;
	G_SetMovedir (self.s.angles, self.movedir);
	self.noise_index = gi.soundindex ("weapons/laser2.wav");

	if (!self.dmg)
		self.dmg = 15;
	if (!self.speed)
		self.speed = 1000;

	self.svflags = SVF_NOCLIENT;
}

//==========================================================

/*QUAKED target_laser (0 .5 .8) (-8 -8 -8) (8 8 8) START_ON RED GREEN BLUE YELLOW ORANGE FAT
When triggered, fires a laser.  You can either set a target
or a direction.
*/
static void target_laser_think (edict_t &self)
{
	if (self.enemy)
	{
		const vec3_t last_movedir = self.movedir;
		const vec3_t point = self.enemy->absmin + (self.enemy->size * 0.5f);
		self.movedir = point - self.s.origin;
		self.movedir.Normalize();

		if (self.movedir != last_movedir)
			self.spawnflags |= 0x80000000;
	}

	edict_ref ignore = self;
	vec3_t start = self.s.origin;
	const vec3_t end = start + (self.movedir * 2048);
	trace_t tr;

	while(1)
	{
		tr = gi.trace (start, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		// hurt it if we can
		if (tr.ent->takedamage)
			T_Damage (tr.ent, self, self.activator, self.movedir, tr.endpos, vec3_origin, self.dmg, 1, DAMAGE_ENERGY);

		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && !tr.ent->client)
		{
			if (self.spawnflags & 0x80000000)
			{
				self.spawnflags &= ~0x80000000;
				gi.WriteByte (SVC_TEMP_ENTITY);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (8);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self.s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
			}

			break;
		}

		ignore = tr.ent;
		start = tr.endpos;
	}

	self.s.old_origin = tr.endpos;
	self.nextthink = level.time + FRAME_MS;
}

static void target_laser_on (edict_t &self)
{
	if (!self.activator)
		self.activator = self;
	self.spawnflags |= 0x80000001;
	self.svflags &= ~SVF_NOCLIENT;
	target_laser_think (self);
}

static void target_laser_off (edict_t &self)
{
	self.spawnflags &= ~1;
	self.svflags |= SVF_NOCLIENT;
	self.nextthink = 0;
}

static void target_laser_use (edict_t &self, edict_t &other, edict_t &activator)
{
	self.activator = activator;
	if (self.spawnflags & 1)
		target_laser_off (self);
	else
		target_laser_on (self);
}

static void target_laser_start (edict_t &self)
{
	self.movetype = MOVETYPE_NONE;
	self.solid = SOLID_NOT;
	self.s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self.s.modelindex = MODEL_WORLD;			// must be non-zero

	// set the beam diameter
	if (self.spawnflags & 64)
		self.s.frame = 16;
	else
		self.s.frame = 4;

	// set the color
	if (self.spawnflags & 2)
		self.s.skinnum = 0xf2f2f0f0;
	else if (self.spawnflags & 4)
		self.s.skinnum = 0xd0d1d2d3;
	else if (self.spawnflags & 8)
		self.s.skinnum = 0xf3f3f1f1;
	else if (self.spawnflags & 16)
		self.s.skinnum = 0xdcdddedf;
	else if (self.spawnflags & 32)
		self.s.skinnum = 0xe0e1e2e3;

	if (!self.enemy)
	{
		if (self.target)
		{
			edict_ref ent = G_Find (nullptr, FOFS(targetname), self.target);
			if (!ent)
				gi.dprintf ("%s at %s: %s is a bad target\n", self.classname, vtos(self.s.origin), self.target);
			self.enemy = ent;
		}
		else
			G_SetMovedir (self.s.angles, self.movedir);
	}

	self.use = target_laser_use;
	self.think = target_laser_think;

	if (!self.dmg)
		self.dmg = 1;

	self.mins = { -8.f, -8.f, -8.f };
	self.maxs = { 8.f, 8.f, 8.f };
	self.Link();

	if (self.spawnflags & 1)
		target_laser_on (self);
	else
		target_laser_off (self);
}

void SP_target_laser (edict_t &self)
{
	// let everything else get spawned before we start firing
	self.think = target_laser_start;
	self.nextthink = level.time + 100;
}

//==========================================================

/*QUAKED target_earthquake (1 0 0) (-8 -8 -8) (8 8 8)
When triggered, this initiates a level-wide earthquake.
All players and monsters are affected.
"speed"		severity of the quake (default:200)
"count"		duration of the quake (default:5)
*/
static void target_earthquake_think (edict_t &self)
{
	if (self.last_move_time < level.time)
	{
		self.PlayPositionedSound (self.noise_index, self.s.origin);
		self.last_move_time = level.time + 500;
	}

	for (auto &e : game.entities.range(1))
	{
		if (!e.inuse)
			continue;
		if (!e.client)
			continue;
		if (!e.groundentity)
			continue;

		e.groundentity = nullptr;
		e.velocity[0] += crandom() * 150;
		e.velocity[1] += crandom() * 150;
		e.velocity[2] = self.speed * (100.0f / e.mass);
	}

	if (level.time < self.timestamp)
		self.nextthink = level.time + FRAME_MS;
}

static void target_earthquake_use (edict_t &self, edict_t &other, edict_t &activator)
{
	self.timestamp = level.time + (self.count * 1000);
	self.nextthink = level.time + FRAME_MS;
	self.activator = activator;
	self.last_move_time = 0;
}

void SP_target_earthquake (edict_t &self)
{
	if (!self.targetname)
		gi.dprintf("untargeted %s at %s\n", self.classname, vtos(self.s.origin));

	if (!self.count)
		self.count = 5;

	if (!self.speed)
		self.speed = 200;

	self.svflags |= SVF_NOCLIENT;
	self.think = target_earthquake_think;
	self.use = target_earthquake_use;

	self.noise_index = gi.soundindex ("world/quake.wav");
}
