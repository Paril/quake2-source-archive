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

bool M_FidgetCheck (edict_t &ent, const int &percent)
{
	//if (ent.control && ((ent.control->client->latched_buttons | ent.control->client->buttons) & BUTTON_ATTACK))
	//	return true;

	return ent.control && prandom(percent);
}

void M_CheckGround (edict_t &ent)
{
	if (ent.velocity[2] > 100)
	{
		ent.groundentity = nullptr;
		return;
	}

	constexpr vec3_t one_quarter_unit_down = { 0.f, 0.f, 0.25f };

	// if the hull point one-quarter unit down is solid the entity is on ground
	const vec3_t point = ent.s.origin - one_quarter_unit_down;

	const trace_t trace = gi.trace (ent.s.origin, ent.mins, ent.maxs, point, ent, MASK_MONSTERSOLID | CONTENTS_MONSTER);

	// check steepness
	if (trace.plane.normal[2] < 0.7f && !trace.startsolid)
	{
		ent.groundentity = nullptr;
		return;
	}

	ent.groundentity = trace.ent;
	ent.groundentity_linkcount = trace.ent->linkcount;
	if (!trace.startsolid && !trace.allsolid)
		ent.s.origin = trace.endpos;
/*	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy (trace.endpos, ent->s.origin);
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0;
	}*/
}

void M_CatagorizePosition (edict_t &ent)
{
	//
	// get waterlevel
	//
	vec3_t point { ent.s.origin[0], ent.s.origin[1], ent.s.origin[2] + ent.mins[2] + 1.f };
	brushcontents_t cont = gi.pointcontents (point);

	if (!(cont & MASK_WATER))
	{
		ent.waterlevel = WATER_NONE;
		ent.watertype = CONTENTS_NONE;
		return;
	}

	ent.watertype = cont;
	ent.waterlevel = WATER_FEET;
	point[2] += 26;
	cont = gi.pointcontents (point);
	if (!(cont & MASK_WATER))
		return;

	ent.waterlevel = WATER_WAIST;
	point[2] += 22;
	cont = gi.pointcontents (point);
	if (cont & MASK_WATER)
		ent.waterlevel = WATER_UNDER;
}

static void M_WorldEffects (edict_t &ent)
{
	if (ent.waterlevel == WATER_NONE)
	{
		if (ent.flags & FL_INWATER)
		{	
			ent.PlaySound(gi.soundindex("player/watr_out.wav"), CHAN_BODY);
			ent.flags &= ~FL_INWATER;
		}

		return;
	}

	if (ent.watertype & CONTENTS_LAVA)
	{
		if (ent.damage_debounce_time < level.time)
		{
			ent.damage_debounce_time = level.time + 200;
			T_Damage (ent, game.world(), game.world(), vec3_origin, ent.s.origin, vec3_origin, 10 * ent.waterlevel, 0, DAMAGE_NONE);
		}
	}

	if (ent.watertype & CONTENTS_SLIME)
	{
		if (ent.damage_debounce_time < level.time)
		{
			ent.damage_debounce_time = level.time + 100;
			T_Damage (ent, game.world(), game.world(), vec3_origin, ent.s.origin, vec3_origin, 4 * ent.waterlevel, 0, DAMAGE_NONE);
		}
	}
	
	if (!(ent.flags & FL_INWATER))
	{	
		if (!(ent.svflags & SVF_DEADMONSTER))
		{
			if (ent.watertype & CONTENTS_LAVA)
			{
				if (prandom(50))
					ent.PlaySound(gi.soundindex("player/lava1.wav"), CHAN_BODY);
				else
					ent.PlaySound(gi.soundindex("player/lava2.wav"), CHAN_BODY);
			}
			else if (ent.watertype & CONTENTS_SLIME)
				ent.PlaySound(gi.soundindex("player/watr_in.wav"), CHAN_BODY);
			else if (ent.watertype & CONTENTS_WATER)
				ent.PlaySound(gi.soundindex("player/watr_in.wav"), CHAN_BODY);
		}

		ent.flags |= FL_INWATER;
		ent.damage_debounce_time = 0;
	}
}

void M_droptofloor (edict_t &ent)
{
	ent.s.origin[2] += 1;

	const vec3_t end = ent.s.origin - vec3_t { 0.f, 0.f, 256.f };
	const trace_t trace = gi.trace (ent.s.origin, ent.mins, ent.maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1 || trace.allsolid)
		return;

	ent.s.origin = trace.endpos;

	ent.Link();
	M_CheckGround (ent);
	M_CatagorizePosition (ent);
}

static void M_SetEffects (edict_t &ent)
{
	ent.s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN|EF_SPHERETRANS);
	ent.s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE|RF_TRANSLUCENT);

	if (ent.deadflag && !ent.control)
	{
		const gtime_t left = ent.monsterinfo.death_time - level.time;
		
		if (left < 500)
			ent.s.effects |= EF_SPHERETRANS;
		if (left < 1000)
			ent.s.renderfx |= RF_TRANSLUCENT;
	}
}

static void M_MoveFrame (edict_t &self)
{
	const mmove_t *move = self.monsterinfo.currentmove;
	self.nextthink = level.time + FRAME_MS;

	if (self.monsterinfo.nextframe && (self.monsterinfo.nextframe >= move->firstframe) && (self.monsterinfo.nextframe <= move->lastframe))
	{
		self.s.frame = self.monsterinfo.nextframe;
		self.monsterinfo.nextframe = 0;
	}
	else
	{
		if (self.s.frame == move->lastframe)
		{
			// check for death
			if (self.svflags & SVF_DEADMONSTER)
				return;

			if (move->endfunc)
			{
				move->endfunc (self);

				// regrab move, endfunc is very likely to change it
				move = self.monsterinfo.currentmove;
			}
		}

		if (self.s.frame < move->firstframe || self.s.frame > move->lastframe)
			self.s.frame = move->firstframe;
		else
		{
			self.s.frame++;
			if (self.s.frame > move->lastframe)
				self.s.frame = move->firstframe;
		}
	}

	const int32_t index = self.s.frame - move->firstframe;
	const mframe_t *frames = move->frame.begin();

	if (frames[index].aifunc)
		frames[index].aifunc (self, frames[index].dist * self.monsterinfo.scale);

	if (frames[index].thinkfunc)
		frames[index].thinkfunc (self);
}

void Unpossess(edict_t &player)
{
	player.movetype = MOVETYPE_WALK;
	player.solid = SOLID_BBOX;
	player.svflags &= ~SVF_NOCLIENT;
	player.client->ps.pmove.pm_type = PM_NORMAL;
	player.client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	player.viewheight = 22;
	player.velocity.Clear();
	player.s.origin = player.control->s.origin;
	player.s.origin[2] += player.control->mins[2];
	player.s.origin[2] -= player.mins[2];
	
	player.health = player.control->health;
	G_FreeEdict(player.control);
	player.control = nullptr;

	EnsureGoodPosition(player);
	player.Link();

	player.s.event = EV_PLAYER_TELEPORT;
	player.client->jump_sound_debounce = level.time + 100;
}

static edict_ref G_FindEnemyToFollow(edict_t &self)
{
	std::vector<edict_ref> choices;

	for (auto &e : game.entities.range(1))
	{
		if (!e.client && !(e.svflags & SVF_MONSTER))
			continue;

		if (!visible(self, e))
			continue;

		choices.push_back(e);
	}

	if (!choices.size())
		return nullptr;

	return choices[irandom(choices.size() - 1)];
}

static bool M_VisibleToHunters(edict_t &ent)
{
	for (auto &player : game.players)
	{
		if (!player.inuse || !player.client || player.client->resp.team != TEAM_HUNTERS || player.client->resp.spectator)
			continue;

		if (visible(ent, player))
			return true;
	}
	
	return false;
}

static void monster_think (edict_t &self)
{
	if (self.control && level.control_delay < level.time)
	{
		if (self.control->client->cmd.upmove > 0 || self.waterlevel >= WATER_UNDER)
		{
			Unpossess(self.control);
			return;
		}
		else if (self.control->client->cmd.forwardmove || self.control->client->cmd.sidemove)
		{
			const vec_t len = sqrt((self.control->client->cmd.forwardmove * self.control->client->cmd.forwardmove) + (self.control->client->cmd.sidemove * self.control->client->cmd.sidemove));

			if (len <= 200)
			{
				if (self.monsterinfo.currentmove->frame.begin()->aifunc != ai_walk)
					self.monsterinfo.walk(self);
			}
			else
			{
				if (self.monsterinfo.currentmove->frame.begin()->aifunc != ai_run)
					self.monsterinfo.run(self);
			}
		}
	}
	else
	{
		if (self.deadflag && self.monsterinfo.death_time < level.time)
		{
			// send effect
			gi.WriteByte (SVC_MUZZLEFLASH);
			gi.WriteEntity (self);
			gi.WriteByte (MZ_LOGOUT);
			gi.multicast (self.s.origin, MULTICAST_PVS);

			G_FreeEdict(self);
			return;
		}

		if (self.waterlevel >= WATER_UNDER)
			EmplaceMonsterInGoodSpot(self);

		if (level.time < self.monsterinfo.stubborn_check_time)
		{
			self.monsterinfo.stubborn_check_time = level.time + frandom(0, 16000);
			self.monsterinfo.stubborn_time = level.time + frandom(0, 8000);
		}

		if (level.time < self.monsterinfo.follow_time)
		{
			if (!self.monsterinfo.follow_ent || !visible(self, self.monsterinfo.follow_ent))
			{
				self.monsterinfo.follow_ent = nullptr;
				self.monsterinfo.follow_time = 0;
		
				self.monsterinfo.follow_check = level.time + frandom(4000, 24000);
			}
		}
		else
			self.monsterinfo.follow_ent = nullptr;

		if (!self.monsterinfo.follow_ent && self.monsterinfo.follow_check < level.time)
		{
			self.monsterinfo.follow_check = level.time + frandom(4000, 24000);

			if (prandom(20))
			{
				self.monsterinfo.follow_ent = G_FindEnemyToFollow(self);

				if (self.monsterinfo.follow_ent)
				{
					self.monsterinfo.follow_direction = prandom(50);
					self.monsterinfo.follow_time = level.time + frandom(8000, 48000);
				}
			}
		}

		const bool hunter_visible = M_VisibleToHunters(self);
		
		if (self.monsterinfo.hunter_visible != hunter_visible)
		{
			if (prandom(35))
				self.monsterinfo.next_runwalk_check = self.monsterinfo.should_stand_check = 0;
			self.monsterinfo.hunter_visible = hunter_visible;
		}
	}

	M_MoveFrame (self);
	if (self.linkcount != self.monsterinfo.linkcount)
	{
		self.monsterinfo.linkcount = self.linkcount;
		M_CheckGround (self);
	}
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);
}

//============================================================================

static void monster_die(edict_t &self, edict_t &inflictor, edict_t &attacker, const int32_t &damage, const vec3_t &point)
{
	if (self.control)
	{
		edict_t &player = self.control;
		Unpossess(player);
		player.die(player, inflictor, attacker, damage, point);
		return;
	}

	if (self.deadflag)
		return;

	for (auto it = level.monsters.begin(); it != level.monsters.end(); it++)
	{
		if (*it == self)
		{
			level.monsters.erase(it);
			break;
		}
	}

	self.monsterinfo.die(self);
	self.deadflag = true;
	self.takedamage = false;
	self.svflags |= SVF_DEADMONSTER;
	self.solid = SOLID_NOT;
	self.monsterinfo.death_time = level.time + frandom(3000, 4000);

	self.Link();

	if (attacker.client)
		gi.bprintf(PRINT_HIGH, "%s has killed an innocent Stroggo!\n", attacker.client->pers.netname);
	//else
	//	gi.bprintf(PRINT_HIGH, "A monster has died from unnatural causes.\n");
}

static void monster_touch (edict_t &self, edict_t &other, const cplane_t *plane, const csurface_t *surf)
{
	if (!other.groundentity || other.groundentity == self)
		return;

	const vec_t ratio = static_cast<vec_t>(other.mass) / self.mass;
	const vec3_t v = self.s.origin - other.s.origin;
	M_walkmove (self, v.ToYaw(), 20 * ratio * FRAME_S);
}

static void monster_start (edict_t &self)
{
	self.mins[0] = self.mins[1] = -16;
	self.maxs[0] = self.maxs[1] = 16;

	const vec_t height = self.maxs[2] - self.mins[2];

	if (height > 56)
		self.maxs[2] -= height - 56;

	self.Link();

	self.nextthink = level.time + FRAME_MS;
	self.svflags |= SVF_MONSTER;
	self.takedamage = true;
	self.air_finished = level.time + 1200;
	self.max_health = self.health;
	self.clipmask = MASK_MONSTERSOLID;

	self.deadflag = false;
	self.svflags &= ~SVF_DEADMONSTER;

	self.die = monster_die;
	self.touch = monster_touch;

	self.s.old_origin = self.s.origin;

	// randomize what frame they start on
	if (self.monsterinfo.currentmove)
		self.s.frame = irandom(self.monsterinfo.currentmove->firstframe, self.monsterinfo.currentmove->lastframe);

	const bool damaged = prandom(50);

	self.health = self.max_health = DEFAULT_HEALTH;

	if (damaged)
	{
		self.health *= 0.5f;
		self.s.skinnum = self.monsterinfo.damaged_skin;
	}
}

static void monster_start_go (edict_t &self)
{
	if (self.health <= 0)
		return;

	self.monsterinfo.stand (self);
	self.think = monster_think;
	self.nextthink = level.time + FRAME_MS;
}

static void walkmonster_start_go (edict_t &self)
{
	M_droptofloor (self);

	if (self.groundentity)
		if (!M_walkmove (self, 0, 0))
			gi.dprintf ("%s in solid at %s\n", self.classname, vtos(self.s.origin));
	
	if (!self.yaw_speed)
		self.yaw_speed = 20;
	if (!self.viewheight)
		self.viewheight = self.maxs[2] - 4;

	monster_start_go (self);
}

void walkmonster_start (edict_t &self)
{
	monster_start(self);
	walkmonster_start_go(self);
}
